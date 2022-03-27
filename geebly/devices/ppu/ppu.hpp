// PPU To-do: Split timing and rendering implementations in separate files

#pragma once

#include "../../log.hpp"

#include "memory.hpp"

#include "../clock.hpp"
#include "../ic.hpp"

#define GEEBLY_OPTIMIZE_PPU

#ifdef GEEBLY_OPTIMIZE_PPU
#include "immintrin.h"
#define PPU_PIXEL_EXTRACT _pext_u32(((u16)h << 8) | l, 0x0101 << p)
#define PPU_SPRITE_PIXEL_EXTRACT _pext_u32(((u16)spr_h << 8) | spr_l, 0x0101 << spr_p)
#else
#define PPU_PIXEL_EXTRACT ((l >> p) & 0x1) | ((h >> p) & 0x1) << 1))
#define PPU_SPRITE_PIXEL_EXTRACT ((spr_l >> spr_p) & 0x1) | ((spr_h >> spr_p) & 0x1) << 1))
#endif

#include <queue>
#include <array>
#include <cstdint>
#include <algorithm>

namespace gameboy {
    namespace ppu {
        /**
		 *  \brief Initialize the PPU
		 *  
		 *  \param fr_cb A function that will be called every time a frame is ready.
         *               Pass nullptr to disable the callback feature
		 */
        void init(frame_ready_callback_t fr_cb = nullptr) {
            frame_ready_cb = fr_cb;

            r[PPU_STAT] = 0x84;
            r[PPU_BGP]  = 0xfc;
            r[PPU_OBP0] = 0xff;
            r[PPU_OBP1] = 0xff;

            // Vblank is fired when first turning on the LCD
            ic::irq |= 0x1;

            for (auto& b : cgb_bg_palette) { b = rand() % 0xff; }
            for (auto& b : cgb_spr_palette) { b = rand() % 0xff; }
            for (auto& b : vram[0]) { b = rand() % 0xff; }
            for (auto& b : vram[1]) { b = rand() % 0xff; }
            for (auto& b : oam) { b = rand() % 0xff; }

            for (size_t idx = 0; idx < sprites.size(); idx++)
                sprites.at(idx) = (sprite_t*)(&oam.at(idx << 2));

            queued_sprites.reserve(12);

            if (settings::skip_bootrom) {
                _log(debug, "Initializing PPU state skipping bootrom");
                r[PPU_LCDC] = 0x91;
                r[PPU_STAT] = 0x85;
                r[PPU_BGP] = 0xfc;
            }
        }

#define WINDOW_ENABLED (TEST_REG(PPU_LCDC, LCDC_WNDSWI))

        inline bool window_visible(size_t x) {
            // _log(debug, "x=%u, wx=%u, wx-7=%i", x, r[PPU_WX], (int)r[PPU_WX] - 7);
            return WINDOW_ENABLED &&
                   (r[PPU_LY] >= r[PPU_WY]) &&
                   (x >= ((int)r[PPU_WX] - 7));
        }

        void refetch(bool window = false) {
            u8 scroll_x     = window ? 0 : r[PPU_SCX],
               scroll_y     = window ? 0 : r[PPU_SCY],
               tilemap_mask = window ? LCDC_WNDTMS : LCDC_BGWTMS,
               switch_mask  = window ? LCDC_WNDSWI : LCDC_BGWSWI;

            if (!TEST_REG(PPU_LCDC, switch_mask)) {
                if (!settings::cgb_mode) {
                    l = 0x0;
                    h = 0x0;

                    return;
                }
            }

            sx = window ? (cx - (r[PPU_WX] - 7)) : (cx + scroll_x);
            sy = window ? wiy : ((r[PPU_LY] + scroll_y) & 0xff);

            tile_scx_off = (sx >> 3) & 0x1f;
            tile_scy_off = ((sy >> 3) << 5);

            u16 tile_off = (TEST_REG(PPU_LCDC, tilemap_mask) ? 0x1c00 : 0x1800) + tile_scx_off + tile_scy_off;

            tile = vram[0][tile_off];

            if (settings::cgb_mode) {
                u8 bg_attr_byte = vram[1][tile_off];

                bg_attr.bgp       = (bg_attr_byte & 0x7);
                bg_attr.vram_bank = (bool)(bg_attr_byte & 0x8);
                bg_attr.xflip     = (bool)(bg_attr_byte & 0x20);
                bg_attr.yflip     = (bool)(bg_attr_byte & 0x40);
                bg_attr.priority  = (bool)(bg_attr_byte & 0x80);
            }

            size_t yflip_off = (bg_attr.yflip ? ((7 - (sy % 8)) << 1) : ((sy % 8) << 1));
    
            if (TEST_REG(PPU_LCDC, LCDC_BGWTSS)) {
                coff = (tile << 4) + yflip_off;
            } else {
                coff = 0x1000 + ((int8_t)tile * 16) + yflip_off;
            }

            size_t bank = settings::cgb_mode ? ((int)bg_attr.vram_bank) : 0;

            l = vram[bank][coff];
            h = vram[bank][coff + 1];
        }

        void render_sprites() {
            bool pixel_pushed = false;
            u16 spr_coff;
            u8 spr_h, spr_l, spr_c;
            size_t spr_p;

            for (sprite_t& spr : queued_sprites) {
                if (((int)cx >= ((int)spr.x - 8)) && ((int)cx < (int)spr.x) && (!pixel_pushed)) {
                    size_t yoff = (spr.a & 0x40) ? 
                        ((TEST_REG(PPU_LCDC, LCDC_SPSIZE) ? 15 : 7) - (r[PPU_LY] - (spr.y - 16))) :
                        (r[PPU_LY] - (spr.y - 16));

                    spr_coff = ((spr.t & (TEST_REG(PPU_LCDC, LCDC_SPSIZE) ? 0xfe : 0xff)) << 4) + (yoff << 1);

                    size_t bank = settings::cgb_mode ? ((spr.a >> 3) & 0x1) : 0;

                    spr_l = vram[bank][spr_coff];
                    spr_h = vram[bank][spr_coff + 1];

                    spr_p = ((int)cx - ((int)spr.x - 8));
                    spr_p = (spr.a & 0x20) ? spr_p : (7 - spr_p);

                    spr_c = PPU_SPRITE_PIXEL_EXTRACT;

                    if (spr_c) pixel_pushed = true;

                    if (pixel_pushed) sprite_fifo.push({
                        spr_c,
                        settings::cgb_mode ? (uint8_t)(spr.a & 0x7) : (uint8_t)((spr.a >> 4) & 0x1),
                        0,
                        (spr.a & 0x80) != 0
                    });
                }
            }

            if (!pixel_pushed) sprite_fifo.push({0, 0, 0, 0});
        }

        void render_row(size_t size) {
            size--;

            u8 scroll_x = r[PPU_SCX],
               scroll_y = r[PPU_SCY];

            refetch(window_visible(cx));

            do {
                if ((!((sx + 1) % 8))) refetch(window_visible(cx));
                if (window_visible(cx)) refetch(true);

                sx = window_visible(cx) ? (cx - (r[PPU_WX] - 7)) : (cx + r[PPU_SCX]);
                sy = (r[PPU_LY] + scroll_y) & 0xff;

                size_t p = bg_attr.xflip ? (sx % 8) : (7 - (sx % 8));

                u8 color = PPU_PIXEL_EXTRACT;

                background_fifo.push(fifo_pixel_t{
                    color,
                    bg_attr.bgp,
                    0,
                    false
                });

                render_sprites();

                cx++;
            } while (size--);
        }

        #define SWITCH_MODE(m) r[PPU_STAT] = (r[PPU_STAT] & ~(STAT_CRMODE)) | m

        bool already_fetched_sprites = false,
             residual_cycles = false;

        u32 get_pixel_color(const fifo_pixel_t& pixel, bool sprite) {
            cgb_palette_ram_t* palette_ram = sprite ? &cgb_spr_palette : &cgb_bg_palette;

            u8 off = (pixel.palette << 3) + (pixel.color << 1);

            u16 p = palette_ram->at(off) | (palette_ram->at(off+1) << 8);

            // Naive CGB color translation
            // u8 r = (p & 0x1f) << 3,
            //    g = ((p >> 0x5) & 0x1f) << 3,
            //    b = ((p >> 0xa) & 0x1f) << 3;

            // Emulate CGB LCD color translation
            u8 r = p & 0x1f,
               g = (p >> 0x5) & 0x1f,
               b = (p >> 0xa) & 0x1f;

            int cr = ((r * 26) + (g *  4) + (b *  2)),
                cg = (           (g * 24) + (b *  8)),
                cb = ((r *  6) + (g *  4) + (b * 22));

            cr = std::min(960, cr) >> 2;
            cg = std::min(960, cg) >> 2;
            cb = std::min(960, cb) >> 2;

            return lgw::rgb(cr, cg, cb);
        }

        unsigned int mode3_length_addend = 0;

        inline void fetch_sprites() {
            static const int mode3_initial_addends[] = {
                3, 2, 2, 2, 2, 1, 1, 1
            };

            if ((!already_fetched_sprites) && (TEST_REG(PPU_LCDC, LCDC_SPDISP))) {
                for (sprite_t* spr : sprites) {
                    // Enforce 10 sprites per scanline limit
                    if (queued_sprites.size() == 10) break;

                    bool sprite_in_scanline =
                        (r[PPU_LY] >= ((int)spr->y - 16)) &&
                        (r[PPU_LY] < ((int)spr->y - (TEST_REG(PPU_LCDC, LCDC_SPSIZE) ? 0 : 8)));

                    if (((int)spr->y-16 >= -16) && sprite_in_scanline) queued_sprites.push_back(*spr);
                }

                already_fetched_sprites = true;

                std::sort(queued_sprites.begin(), queued_sprites.end(), [](sprite_t& a, sprite_t& b) {
                    return a.x < b.x;
                });

                if (queued_sprites.size() >= 1) {
                    mode3_length_addend = mode3_initial_addends[queued_sprites[0].x % 8] * 4;
                }

                if (queued_sprites.size() > 1) {
                    for (int i = 1; i < queued_sprites.size(); i++) {
                        int x = queued_sprites[i].x - 8;

                        if (x < 8) {
                            mode3_length_addend += (1 * 4);
                        } else {
                            if ((x % 8) < 4) {
                                mode3_length_addend += (2 * 4);
                            } else {
                                mode3_length_addend += (1 * 4);
                            }
                        }
                    }
                }
            }
        }

        bool stat_irq_signal = false, prev_stat_irq_signal = stat_irq_signal,
             vbl_irq_signal = false, prev_vbl_irq_signal = vbl_irq_signal;

        bool stat_hbl_irq_signal = false, prev_stat_hbl_irq_signal = stat_hbl_irq_signal,
             stat_vbl_irq_signal = false, prev_stat_vbl_irq_signal = stat_vbl_irq_signal,
             stat_md2_irq_signal = false, prev_stat_md2_irq_signal = stat_md2_irq_signal,
             stat_lyc_irq_signal = false, prev_stat_lyc_irq_signal = stat_lyc_irq_signal;
        
        bool md0_mode_signal = false, prev_md0_mode_signal = md0_mode_signal,
             md1_mode_signal = false, prev_md1_mode_signal = md1_mode_signal,
             md2_mode_signal = false, prev_md2_mode_signal = md2_mode_signal,
             md3_mode_signal = false, prev_md3_mode_signal = md3_mode_signal;
        
        bool fake_stat_irq = false, prev_fake_stat_irq = fake_stat_irq;
        
        void test_irqs() {
            prev_stat_irq_signal = stat_irq_signal;
            prev_vbl_irq_signal = vbl_irq_signal;
            prev_stat_hbl_irq_signal = stat_hbl_irq_signal;
            prev_stat_vbl_irq_signal = stat_vbl_irq_signal;
            prev_stat_md2_irq_signal = stat_md2_irq_signal;
            prev_stat_lyc_irq_signal = stat_lyc_irq_signal;

            prev_md0_mode_signal = md0_mode_signal;
            prev_md1_mode_signal = md1_mode_signal;
            prev_md2_mode_signal = md2_mode_signal;
            prev_md3_mode_signal = md3_mode_signal;

            prev_fake_stat_irq = fake_stat_irq;

            md0_mode_signal = (r[PPU_STAT] & 0x3) == 0;
            md1_mode_signal = (r[PPU_STAT] & 0x3) == 1;
            md2_mode_signal = (r[PPU_STAT] & 0x3) == 2;
            md3_mode_signal = (r[PPU_STAT] & 0x3) == 3;

            //_log(debug, "ly=%02x, lyc=%02x, signal=%u, prev=%u", r[PPU_LY], r[PPU_LYC], ((r[PPU_LY] == r[PPU_LYC]) && TEST_REG(PPU_STAT, STAT_LYCNSD)), prev_stat_irq_signal);

            stat_lyc_irq_signal = ((r[PPU_LY] == r[PPU_LYC]) && TEST_REG(PPU_STAT, STAT_LYCNSD));
            stat_hbl_irq_signal = (((r[PPU_STAT] & 0x3) == 0) && TEST_REG(PPU_STAT, STAT_MODE00));
            stat_md2_irq_signal = (((r[PPU_STAT] & 0x3) == 2) && TEST_REG(PPU_STAT, STAT_MODE02));
            stat_vbl_irq_signal = (((r[PPU_STAT] & 0x3) == 1) && (TEST_REG(PPU_STAT, STAT_MODE01)));

            stat_irq_signal = (stat_lyc_irq_signal) ||
                              (stat_hbl_irq_signal) ||
                              (stat_md2_irq_signal) ||
                              (stat_vbl_irq_signal);

            // stat_irq_signal = ((stat_lyc_irq_signal == true) && (prev_stat_lyc_irq_signal == false)) ||
            //                   ((stat_hbl_irq_signal == true) && (prev_stat_hbl_irq_signal == false)) ||
            //                   ((stat_md2_irq_signal == true) && (prev_stat_md2_irq_signal == false)) ||
            //                   ((stat_vbl_irq_signal == true) && (prev_stat_vbl_irq_signal == false));

            vbl_irq_signal = (r[PPU_STAT] & 0x3) == 1;

            bool lcd_on = TEST_REG(PPU_LCDC, LCDC_SWITCH);

            if (stat_irq_signal && (!prev_stat_irq_signal) && lcd_on) ic::fire(IRQ_STAT);
            if (vbl_irq_signal && (!prev_vbl_irq_signal) && lcd_on) ic::fire(IRQ_VBL);

            if (r[PPU_LY] == r[PPU_LYC]) {
                r[PPU_STAT] |= STAT_CDFLAG; 
            } else {
                r[PPU_STAT] &= ~STAT_CDFLAG;
            }
        }

#define MODE3_BASE_LENGTH 160
#define MODE2_LENGTH 80
#define MODE0_BASE_LENGTH (456 - MODE2_LENGTH - MODE3_BASE_LENGTH)

        bool& toggle(bool& var) { return var = !var; }

        void cycle() {
            if (!TEST_REG(PPU_LCDC, LCDC_SWITCH)) {
                r[PPU_LY] = 0x0;
                clk = 0;
            }

            switch (r[PPU_STAT] & STAT_CRMODE) {
                case MODE_SPR_SEARCH: {
                    oam_disabled = true;

                    if (residual_cycles) {
                        clki += clk;
                        residual_cycles = false;
                    }

                    // Queue up 10 sprites max for this scanline
                    fetch_sprites();

                    if (pause)
                        _log(debug, "mode 2 cycles remaining for switch =%i", MODE2_LENGTH - clk);

                    if (clk >= MODE2_LENGTH) {
                        // Reset sprite fetcher state for the next scanline
                        already_fetched_sprites = false;

                        clk -= MODE2_LENGTH;

                        if (clk) residual_cycles = true;

                        SWITCH_MODE(MODE_DRAW);
                    }
                } break;

                case MODE_DRAW: {
                    vram_disabled = true;

                    // Handle pixels drawn in residual cycles (solves black lines bug)
                    unsigned int pcount = clki;

                    if (residual_cycles) {
                        pcount += clk;
                        residual_cycles = false;
                    }

                    render_row(pcount);

                    // Pop sprite and background/window pixels from the FIFO for mixing
                    // given priorities, colors, etc.
                    for (size_t i = 0; i < pcount; i++) {
                        if (!background_fifo.size()) break;

                        fifo_pixel_t bg_pixel = background_fifo.front(),
                                     spr_pixel = sprite_fifo.front();

                        u8 bg_idx = 0x0, spr_idx = 0x0;
                        u32 out = 0x0;

                        if (!TEST_REG(PPU_LCDC, LCDC_SPDISP)) spr_pixel.color = 0;

                        if (settings::cgb_mode) {
                            if (settings::cgb_dmg_mode) {
                                spr_pixel.color = ((spr_pixel.palette ? r[PPU_OBP1] : r[PPU_OBP0]) >> (spr_pixel.color << 1)) & 0x3;
                                bg_pixel.color = (r[PPU_BGP] >> (bg_pixel.color << 1)) & 0x3;
                            }

                            u32 bg_out = get_pixel_color(bg_pixel, false),
                                spr_out = get_pixel_color(spr_pixel, true);

                            if (spr_pixel.bg_priority && TEST_REG(PPU_LCDC, LCDC_BGWSWI)) {
                                out = bg_pixel.color ? bg_out : spr_out;
                            } else {
                                out = spr_pixel.color ? spr_out : bg_out;
                            }

                        } else {
                            if (!TEST_REG(PPU_LCDC, LCDC_BGWSWI)) bg_pixel.color = 0;

                            spr_idx = ((spr_pixel.palette ? r[PPU_OBP1] : r[PPU_OBP0]) >> (spr_pixel.color << 1)) & 0x3;
                            bg_idx = (r[PPU_BGP] >> (bg_pixel.color << 1)) & 0x3;

                            if (spr_pixel.bg_priority) {
                                out = bg_pixel.color ? dmg_palette.at(bg_idx) : dmg_palette.at(spr_idx);
                            } else {
                                out = spr_pixel.color ? dmg_palette.at(spr_idx) : dmg_palette.at(bg_idx);
                            }
                        }

                        if (stopped) {
                            if (settings::cgb_mode) {
                                frame[buffer_latch].draw(fx++, r[PPU_LY], 0x000000ff);
                            } else {
                                frame[buffer_latch].draw(fx++, r[PPU_LY], 0xffffffff);
                            }
                        } else if (!TEST_REG(PPU_LCDC, LCDC_SWITCH)) {
                            frame[buffer_latch].draw(fx++, r[PPU_LY], 0xffffffff);
                        } else {
                            if (fx < PPU_WIDTH)
                                frame[buffer_latch].draw(fx++, r[PPU_LY], out);
                        }

                        if (sprite_fifo.size()) sprite_fifo.pop(); else break;
                        if (background_fifo.size()) background_fifo.pop(); else break;
                    }

                    if (pause)
                        _log(debug, "mode 3 cycles remaining for switch =%i", MODE3_BASE_LENGTH - clk);

                    if (clk >= (MODE3_BASE_LENGTH + mode3_length_addend)) {
                        // Prepare the sprite FIFO for the next scanline
                        queued_sprites.clear();

                        clk -= MODE3_BASE_LENGTH + mode3_length_addend;

                        if (clk) residual_cycles = true;

                        vram_disabled = false;
                        oam_disabled = false;

                        if (pause)
                            _log(debug, "switching to hblank cycles=%i", clk);

                        SWITCH_MODE(MODE_HBLANK);
                    }
                } break;

                case MODE_HBLANK: {
                    //if (clk >= 204) {
                    if (clk >= (MODE0_BASE_LENGTH - mode3_length_addend)) {
                        if ((r[PPU_LY] >= r[PPU_WY]) && TEST_REG(PPU_LCDC, LCDC_WNDSWI) && ((r[PPU_WX] - 7) <= PPU_WIDTH)) wiy++;

                        if (TEST_REG(PPU_LCDC, LCDC_SWITCH)) r[PPU_LY]++; else r[PPU_LY] = 0;

                        // clk -= 204;
                        clk -= MODE0_BASE_LENGTH - mode3_length_addend;

                        residual_cycles = clk;

                        mode3_length_addend = 0;

                        if (r[PPU_LY] == 144) {
                            SWITCH_MODE(MODE_VBLANK);

                            if (frame_ready_cb != nullptr)
                                frame_ready_cb(frame[buffer_latch].get_buffer(), frame[!buffer_latch].get_buffer());
                            
                            toggle(buffer_latch);
                        } else {
                            fx = 0;
                            cx = 0;

                            SWITCH_MODE(MODE_SPR_SEARCH);
                        }
                    }
                } break;

                case MODE_VBLANK: {
                    switch (r[PPU_LY]) {
                        default: {
                            if (clk >= 456) {
                                clk -= 456;

                                r[PPU_LY]++;
                            }
                        } break;

                        case 153: {
                            if (clk >= 4) {
                                clk -= 4;

                                wiy = 0;
                                r[PPU_LY] = 0;
                                fx = 0;
                                cx = 0;
                                //clk = 0;

                                //SWITCH_MODE(MODE_SPR_SEARCH);
                            }
                        } break;
                        case 0: {
                            if (clk >= 452) {
                                clk -= 452;

                                wiy = 0;
                                r[PPU_LY] = 0;
                                fx = 0;
                                cx = 0;

                                SWITCH_MODE(MODE_SPR_SEARCH);
                            }
                        } break;
                    }
                    // if (clk >= 456) {
                    //     if (TEST_REG(PPU_LCDC, LCDC_SWITCH))
                    //         r[PPU_LY]++; else r[PPU_LY] = 0;

                    //     clk -= 456;

                    //     if (clk) residual_cycles = true;

                    //     if (r[PPU_LY] == 153) {
                    //         SWITCH_MODE(MODE_LY153);
                    //     }
                    // }
                } break;
            }

            test_irqs();

            if (TEST_REG(PPU_LCDC, LCDC_SWITCH)) {
                clki = clock::get();

                clk += clki;
            } else {
                r[PPU_LY] = 0x0;
                clk = 0;
            }

            // This behavior makes VRAM/OAM Blocked Writes not pass
            // if (trigger_oam_bug) {
            //     if (!oam_disabled)
            //         for (auto& b : oam) { b = rand() % 0xff; }

            //     trigger_oam_bug = false;
            // }
        }
    }
}

#undef TEST_REG