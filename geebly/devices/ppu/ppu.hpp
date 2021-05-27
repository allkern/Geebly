// PPU To-do: Split timing and rendering implementations in separate files

#pragma once

#include "../../log.hpp"

#include "memory.hpp"

#include "../clock.hpp"
#include "../ic.hpp"

#include "immintrin.h"

#define GEEBLY_OPTIMIZE_PPU

#ifdef GEEBLY_OPTIMIZE_PPU
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
            ic::ia |= 0x1;

            for (auto& b : cgb_bg_palette) { b = rand() % 0xff; }
            for (auto& b : cgb_spr_palette) { b = rand() % 0xff; }
            for (auto& b : vram[0]) { b = rand() % 0xff; }
            for (auto& b : vram[1]) { b = rand() % 0xff; }
            for (auto& b : oam) { b = rand() % 0xff; }

            for (size_t idx = 0; idx < sprites.size(); idx++)
                sprites.at(idx) = reinterpret_cast<sprite_t*>(&oam.at(idx << 2));

            queued_sprites.reserve(12);

            if (settings::skip_bootrom) {
                r[PPU_LCDC] = 0x91;
                r[PPU_STAT] = 0x85;
                r[PPU_BGP] = 0xfc;
            }
        }

        void refetch(size_t size = 0) {
            bool window = TEST_REG(PPU_LCDC, LCDC_WNDSWI) && (r[PPU_LY] >= r[PPU_WY]) && ((cx - size) >= (r[PPU_WX] - 7));

            u8 scroll_x     = window ? (r[PPU_WX] - 7) : r[PPU_SCX],
               scroll_y     = window ? r[PPU_WY] : r[PPU_SCY],
               tilemap_mask = window ? LCDC_WNDTMS : LCDC_BGWTMS,
               switch_mask  = window ? LCDC_WNDSWI : LCDC_BGWSWI;

            if (!TEST_REG(PPU_LCDC, switch_mask)) {
                l = 0x0;
                h = 0x0;
                return;
            }

            sx = cx + (window ? -scroll_x : scroll_x);
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

            if (TEST_REG(PPU_LCDC, LCDC_BGWTSS)) {
                coff = (tile << 4) + (bg_attr.yflip ? ((7 - (sy % 8)) << 1) : ((sy % 8) << 1));
            } else {
                coff = 0x1000 + ((int8_t)tile * 16) + (bg_attr.yflip ? ((7 - (sy % 8)) << 1) : ((sy % 8) << 1));
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
                        (spr.a & 0x80)
                    });
                }
            }

            if (!pixel_pushed) sprite_fifo.push({0, 0, 0, 0});
        }

        void render_row(size_t size) {
            size--;

            bool window = TEST_REG(PPU_LCDC, LCDC_WNDSWI) && (r[PPU_LY] >= r[PPU_WY]) && ((cx + size) >= (r[PPU_WX] - 7));

            u8 scroll_x = window ? (r[PPU_WX] - 7): r[PPU_SCX],
               scroll_y = window ? r[PPU_WY] : r[PPU_SCY];

            refetch();

            do {
                if (!((sx + 1) % 8) || (window && ((cx + size) == (r[PPU_WX] - 7)))) refetch();

                sx = cx + (window ? -scroll_x : scroll_x);
                sy = window ? wiy : ((r[PPU_LY] + scroll_y) & 0xff);

                size_t p = bg_attr.xflip ? (sx % 8) : (7 - (sx % 8));

                u8 color = PPU_PIXEL_EXTRACT;

                background_fifo.push({
                    color,
                    bg_attr.bgp,
                    0,
                    false
                });

                render_sprites();

                cx++;
            } while (size--);
        }

        bool vbl_triggered = false;

        #define SWITCH_MODE(m) r[PPU_STAT] = (r[PPU_STAT] & ~(STAT_CRMODE)) | m

        bool vbl_pure_fired = false,
             lyc_stat_fired = false,
             oam_stat_fired = false,
             vbl_stat_fired = false,
             hbl_stat_fired = false,
             already_fetched_sprites = false,
             residual_cycles = false;

        u32 get_pixel_color(const fifo_pixel_t& pixel, bool sprite) {
            cgb_palette_ram_t* palette_ram = sprite ? &cgb_spr_palette : &cgb_bg_palette;

            u8 off = (pixel.palette << 3) + (pixel.color << 1);

            u16 p = palette_ram->at(off) | (palette_ram->at(off+1) << 8);

            u8 r = (p & 0x1f) << 3,
               g = ((p >> 0x5) & 0x1f) << 3,
               b = ((p >> 0xa) & 0x1f) << 3;

            return lgw::rgb(r, g, b);
        }

        inline void fire_lyc_irq() {
            if (r[PPU_LY] == r[PPU_LYC]) {
                r[PPU_STAT] |= 0x4;
            } else {
                r[PPU_STAT] &= ~0x4;
            }
            if (TEST_REG(PPU_STAT, STAT_LYCNSD) && (r[PPU_LY] == r[PPU_LYC]) && (!lyc_stat_fired)) {
                ic::ia |= IRQ_STAT;
                lyc_stat_fired = true;
                r[PPU_STAT] |= 0x4;
            }
        }

        inline void fire_oam_irq() {
            if ((!oam_stat_fired) && TEST_REG(PPU_STAT, STAT_MODE02)) {
                ic::ia |= IRQ_STAT;
                oam_stat_fired = true;
            }
        }

        inline void fire_hbl_irq() {
            if ((!hbl_stat_fired) && TEST_REG(PPU_STAT, STAT_MODE00)) {
                ic::ia |= IRQ_STAT;
                hbl_stat_fired = true;
            }
        }

        inline void fire_stat_vbl_irq() {
            if ((!vbl_stat_fired) && TEST_REG(PPU_STAT, STAT_MODE01)) {
                ic::ia |= IRQ_STAT;
                vbl_stat_fired = true;
            }
        }

        inline void fire_pure_vbl_irq() {
            if (!vbl_pure_fired) {
                ic::ia |= IRQ_VBL;
                vbl_pure_fired = true;
            }
        }

        inline void fetch_sprites() {
            if ((!already_fetched_sprites) && (TEST_REG(PPU_LCDC, LCDC_SPDISP))) {
                for (sprite_t* spr : sprites) {
                    // Enforce 10 sprites per scanline limit
                    if (queued_sprites.size() == 10) {
                        std::sort(queued_sprites.begin(), queued_sprites.end(), [](sprite_t& a, sprite_t& b) {
                            return a.x < b.x;
                        });
                        break;
                    }

                    bool sprite_in_scanline =
                        (r[PPU_LY] >= ((int)spr->y - 16)) &&
                        (r[PPU_LY] < ((int)spr->y - (TEST_REG(PPU_LCDC, LCDC_SPSIZE) ? 0 : 8)));

                    if (((int)spr->y-16 >= -16) && sprite_in_scanline) queued_sprites.push_back(*spr);
                }
            }
        }

        void cycle() {
            switch (r[PPU_STAT] & STAT_CRMODE) {
                case MODE_SPR_SEARCH: {
                    oam_disabled = true;

                    // Test IRQs for this scanline
                    fire_lyc_irq();
                    fire_oam_irq();

                    // Queue up 10 sprites max for this scanline
                    fetch_sprites();

                    if (clk >= 80) {
                        // Reset sprite fetcher state for the next scanline
                        already_fetched_sprites = false;

                        oam_stat_fired = false;
                        lyc_stat_fired = false;

                        clk -= 80;

                        if (clk) residual_cycles = true;

                        SWITCH_MODE(MODE_DRAW);
                    }
                } break;

                case MODE_DRAW: {
                    vram_disabled = true;

                    // Handle pixels drawn in residual cycles (solves black lines bug)
                    if (residual_cycles) {
                        clki += clk;
                        residual_cycles = false;
                    }

                    render_row(clki);

                    // Pop sprite and background/window pixels from the FIFO for mixing
                    // given priorities, colors, etc.
                    for (size_t i = 0; i < clki; i++) {
                        fifo_pixel_t bg_pixel = background_fifo.front(),
                                     spr_pixel = sprite_fifo.front();

                        u8 bg_idx = 0x0, spr_idx = 0x0;
                        u32 out = 0x0;

                        if (!TEST_REG(PPU_LCDC, LCDC_SPDISP)) spr_pixel.color = 0;

                        if (settings::cgb_mode) {
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
                        
                        if (fx < PPU_WIDTH) frame.draw(fx++, r[PPU_LY], out);

                        sprite_fifo.pop();
                        background_fifo.pop();
                    }

                    if (clk >= 172) {
                        // Prepare the sprite FIFO for the next scanline
                        queued_sprites.clear();

                        clk -= 172;

                        if (clk) residual_cycles = true;

                        vram_disabled = false;
                        oam_disabled = false;

                        SWITCH_MODE(MODE_HBLANK);
                    }
                } break;

                case MODE_HBLANK: {
                    // Test HBL IRQ
                    fire_hbl_irq();

                    if (clk >= 204) {
                        if ((r[PPU_LY] >= r[PPU_WY]) && TEST_REG(PPU_LCDC, LCDC_WNDSWI) && ((r[PPU_WX] - 7) <= PPU_WIDTH)) wiy++;

                        r[PPU_LY]++;

                        clk -= 204;

                        hbl_stat_fired = false;

                        if (r[PPU_LY] == 144) {
                            SWITCH_MODE(MODE_VBLANK);

                            if (frame_ready_cb != nullptr)
                                frame_ready_cb(frame.get_buffer());

                        } else {
                            fx = 0;
                            cx = 0;

                            SWITCH_MODE(MODE_SPR_SEARCH);
                        }
                    }
                } break;

                case MODE_VBLANK: {
                    // Test VBL IRQs
                    fire_pure_vbl_irq();
                    fire_stat_vbl_irq();
                    fire_lyc_irq();

                    if (clk >= 456) {
                        r[PPU_LY]++;

                        clk -= 456;

                        if (r[PPU_LY] == 154) {
                            vbl_stat_fired = false;
                            vbl_pure_fired = false;
                            lyc_stat_fired = false;
                            
                            wiy = 0;
                            r[PPU_LY] = 0;
                            fx = 0;
                            cx = 0;

                            SWITCH_MODE(MODE_SPR_SEARCH);
                        }
                    }
                } break;
            }

            clki = clock::get();

            clk += clki;
        }
    }
}

#undef TEST_REG