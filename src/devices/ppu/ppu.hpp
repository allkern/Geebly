#pragma once

#include <iostream>

#define LGW_FORMAT_ARGB8888

#include "lgw/framebuffer.hpp"

#include "SDL2/SDL.h"

#include "../../aliases.hpp"
#include "../../global.hpp"
#include "../joypad.hpp"
#include "../clock.hpp"
#include "../ic.hpp"
#include "memory.hpp"

#include <cstdlib>
#include <array>

#ifdef _WIN32
#define GEEBLY_SDL_RENDERER_FLAG SDL_WINDOW_VULKAN
#define GEEBLY_SDL_RENDERER_SETTINGS (SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC)
#endif

#ifdef __linux__
#define GEEBLY_SDL_RENDERER_FLAG SDL_WINDOW_OPENGL
#define GEEBLY_SDL_RENDERER_SETTINGS SDL_RENDERER_PRESENTVSYNC
#endif

#define PPU_WIDTH  160
#define PPU_HEIGHT 144

#define TO_STRING(m) STR(m)
#define STR(m) #m

//#define GEEBLY_NES_MODE

#ifdef GEEBLY_NES_MODE
#define GEEBLY_SDL_RENDERER_SETTINGS SDL_RENDERER_ACCELERATED
#define PPU_WIDTH 0xff
#define PPU_HEIGHT 0xef
#endif

// PPU register indexes
#define PPU_LCDC    0x0
#define PPU_STAT    0x1
#define PPU_SCY     0x2
#define PPU_SCX     0x3
#define PPU_LY      0x4
#define PPU_LYC     0x5
#define PPU_BGP     0x7
#define PPU_OBP0    0x8
#define PPU_OBP1    0x9
#define PPU_WY      0xa
#define PPU_WX      0xb

#define MMIO_VBK    0xff4f

// LCDC masks
#define LCDC_SWITCH 0b10000000
#define LCDC_WNDTMS 0b01000000
#define LCDC_WNDSWI 0b00100000
#define LCDC_BGWTSS 0b00010000
#define LCDC_BGWTMS 0b00001000
#define LCDC_SPSIZE 0b00000100
#define LCDC_SPDISP 0b00000010 
#define LCDC_BGWSWI 0b00000001

// STAT masks
#define STAT_LYCNSD 0b01000000
#define STAT_MODE02 0b00100000
#define STAT_MODE01 0b00010000
#define STAT_MODE00 0b00001000
#define STAT_CDFLAG 0b00000100
#define STAT_CRMODE 0b00000011

// Sprite attribute masks
#define SPATTR_PRIO 0b10000000
#define SPATTR_YFLP 0b01000000
#define SPATTR_XFLP 0b00100000
#define SPATTR_PALL 0b00010000

// CGB mode
#define SPATTR_TVBK 0b00001000
#define SPATTR_PALN 0b00000111

namespace gameboy {
    namespace ppu {
        namespace sdl {
            SDL_Window*   window   = nullptr;
            SDL_Renderer* renderer = nullptr;
            SDL_Texture*  texture  = nullptr;

            bool window_is_open = false;
        }

        lgw::framebuffer <PPU_WIDTH, PPU_HEIGHT> frame;

        u32 color_palette[] = {
            0xfffffffful,
            0xffaaaaaaul,
            0xff555555ul,
            0xff000000ul
        };

        //u32 color_palette[] = {
        //    0xffe0f8d0ul,
        //    0xff88c070ul,
        //    0xff346856ul,
        //    0xff081820ul
        //};

        //u32 color_palette[] = {
        //    0xe8fcccfful,
        //    0xacd490fful,
        //    0x548c70fful,
        //    0x081820fful
        //};

        //u32 color_palette[] = {
        //    0xffefcefful,
        //    0xde944afful,
        //    0xad2921fful,
        //    0x311852fful,
        //};

        // Macro to test register bits
        #define TEST_REG(reg, mask) (r[reg] & mask)

        inline void init_window(size_t scale, bool fullscreen = false) {
            sdl::window = SDL_CreateWindow(
                "Geebly " TO_STRING(GEEBLY_VERSION_TAG) " " TO_STRING(GEEBLY_COMMIT_HASH),
                SDL_WINDOWPOS_UNDEFINED,
                SDL_WINDOWPOS_UNDEFINED,
                PPU_WIDTH * scale, PPU_HEIGHT * scale,
                GEEBLY_SDL_RENDERER_FLAG
            );

            sdl::renderer = SDL_CreateRenderer(
                sdl::window,
                -1,
                GEEBLY_SDL_RENDERER_SETTINGS
            );

            sdl::texture = SDL_CreateTexture(
                sdl::renderer,
                SDL_PIXELFORMAT_ARGB8888,
                SDL_TEXTUREACCESS_STREAMING,
                PPU_WIDTH, PPU_HEIGHT
            );
        }

        // Fix JOYP emulation
        inline void update_window() {
            SDL_Event event;
            
            if (SDL_PollEvent(&event)) {
                switch (event.type) {
                    case SDL_QUIT: { window_closed = true; } break;
                    case SDL_KEYDOWN: { if (!event.key.repeat) joypad::keydown(event.key.keysym.sym); } break;
                    case SDL_KEYUP: { joypad::keyup(event.key.keysym.sym); } break;
                    default: break;
                }
            }
        }

        void init(int scale) {
            SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS);

            init_window(scale);

            // Initialize VRAM bank 0 and 1, OAM, and CGB palettes with random values
            srand(time(NULL));

            for (auto& b : cgb_bg_palette) { b = rand() % 0xff; }
            for (auto& b : cgb_spr_palette) { b = rand() % 0xff; }
            for (auto& b : vram[0]) { b = rand() % 0xff; }
            for (auto& b : vram[1]) { b = rand() % 0xff; }
            for (auto& b : oam) { b = rand() % 0xff; }

            // Start PPU on mode 2 (OAM read)
            r[PPU_STAT] = 0x84;

            if (settings::skip_bootrom) {
                r[PPU_LCDC] = 0x91;
                r[PPU_STAT] = 0x85;
                r[PPU_BGP] = 0xfc;
            }
        }

        inline bool vram_disabled() {
            // Disable VRAM on mode 3, and when LCDC bit 7 is low
            return (r[PPU_STAT] & 3) == 3 && TEST_REG(PPU_LCDC, LCDC_SWITCH);
        }
        
        inline bool oam_disabled() {
            // Disable OAM on modes 2 & 3, and when LCDC bit 7 is low
            return (r[PPU_STAT] & 3) >= 2 && TEST_REG(PPU_LCDC, LCDC_SWITCH);
        }

        struct cgb_bg_attribute {
            u8 bgp;
            bool vram_bank, xflip, yflip, priority;
        } bg_attr;

#ifdef GEEBLY_NES_MODE
        u8 counter = 1;
#endif

        // Maybe should probably implement this as a state machine later on?
        void render_scanline() {
#ifdef GEEBLY_NES_MODE
            if (counter--) {
                return;
            } else {
                counter = 1;
            }
#endif
            bool window = TEST_REG(PPU_LCDC, LCDC_WNDSWI) && (r[PPU_LY] >= r[PPU_WY]);

            u8 sw_mask  = window ? LCDC_WNDSWI  : LCDC_BGWSWI,
               scrolly  = window ? PPU_WY       : PPU_SCY,
               scrollx  = window ? PPU_WX       : PPU_SCX,
               tilemap  = window ? LCDC_WNDTMS  : LCDC_BGWTMS;

            if (TEST_REG(PPU_LCDC, sw_mask)) {
                u8 y = r[PPU_LY] + (window ? (-r[scrolly]) : r[scrolly]);

                // Get tilemap and tileset offsets based off bits 3 and 4 of LCDC
                u16 tilemap_offset = (TEST_REG(PPU_LCDC, tilemap) ? 0x1c00 : 0x1800);
                
                #define TILE_OFFSET (tilemap_offset + ((y >> 3) << 5) + (r[scrollx] >> 3))

                // Signed mode requires a slightly different offset calculation
                #define CHAR_OFFSET \
                    (TEST_REG(PPU_LCDC, LCDC_BGWTSS)) ? \
                        (tile * 16) + ((y % 8) * 2) : \
                        0x1000 + (((s8)tile * 16) + ((y % 8) * 2))
                
                u16 tm_off = TILE_OFFSET;
                u8 tile = vram[0][tm_off];

                if (settings::cgb_mode) {
                    u8 bg_attr_byte = vram[1][TILE_OFFSET];

                    bg_attr = {
                        (u8)(bg_attr_byte & 0x7),
                        (bool)(bg_attr_byte & 0x8),
                        (bool)(bg_attr_byte & 0x20),
                        (bool)(bg_attr_byte & 0x40),
                        (bool)(bg_attr_byte & 0x80)
                    };
                } 

                u16 char_base = CHAR_OFFSET;

                u8 l = vram[settings::cgb_mode ? (int)bg_attr.vram_bank : 0][char_base],
                   h = vram[settings::cgb_mode ? (int)bg_attr.vram_bank : 0][char_base + 1];

                // Start rendering the whole scanline
                for (u8 x = 0; x < PPU_WIDTH; x++) {
                    if (TEST_REG(PPU_LCDC, sw_mask)) {
                        u8 px = 7 - ((x + ((r[scrollx] - (window ? 7 : 0)) % 8)) % 8);

                        u8 pal_offset = ((l >> px) & 1) | (((h >> px) & 1) << 1);

                        int color = 0;
                        u32 out = 0;
                    
                        if (settings::cgb_mode) {
                            color = pal_offset;
                            u8 off = (bg_attr.bgp << 3) + (color << 1);
                            u16 p = cgb_bg_palette[off] | (cgb_bg_palette[off+1] << 8);
                            u8 r = (p & 0x1f) << 3,
                               g = (((p >> 0x5) & 0x1f) << 3),
                               b = (((p >> 0xa) & 0x1f) << 3);
                            out = lgw::rgb(r, g, b);
                        } else {
                            color = (r[PPU_BGP] >> (pal_offset * 2)) & 0x3;
                            out = color_palette[color];
                        }

                        frame.draw(x, r[PPU_LY], out);

                        // Recalculate offsets and settings when reaching tile boundaries
                        if (px == 0) {
                            tilemap_offset++;
                            u16 tm_off = TILE_OFFSET - ((y >> 3) << 5);
                            tile = vram[0][((tm_off & 0xff00) | ((tm_off & 0xff) % 0x20)) + ((y >> 3) << 5)];
                            char_base = CHAR_OFFSET;

                            if (settings::cgb_mode) {
                                u8 bg_attr_byte = vram[1][((tm_off & 0xff00) | ((tm_off & 0xff) % 0x20)) + ((y >> 3) << 5)];

                                bg_attr.bgp       = (bg_attr_byte & 0x7);
                                bg_attr.vram_bank = (bool)(bg_attr_byte & 0x8);
                                bg_attr.xflip     = (bool)(bg_attr_byte & 0x20);
                                bg_attr.yflip     = (bool)(bg_attr_byte & 0x40);
                                bg_attr.priority  = (bool)(bg_attr_byte & 0x80);
                            }

                            l = vram[settings::cgb_mode ? (int)bg_attr.vram_bank : 0][char_base];
                            h = vram[settings::cgb_mode ? (int)bg_attr.vram_bank : 0][char_base + 1];
                        }
                    }
                }
            }
        }

        // Clean this up
        void render_sprites() {
            u16 addr = 0;

            int spsize = TEST_REG(PPU_LCDC, LCDC_SPSIZE) ? 16 : 8;

            vram_bank_t* spr_bank = &vram[0];

            while (addr <= 0x9f) {
                int y_start = 0,
                    y_end = spsize,
                    x_start = 0,
                    x_end = 8;
                
                u8 by = oam[addr++],
                   bx = oam[addr++],
                   t = oam[addr++],
                   attr = oam[addr++];

                //if (settings::cgb_mode && !(attr & SPATTR_TVBK)) spr_bank = &vram[1];

                bool x_flip = (attr & SPATTR_XFLP),
                     y_flip = (attr & SPATTR_YFLP);

                if (x_flip) { std::swap(x_start, x_end); x_start--; x_end--; };
                if (y_flip) { std::swap(y_start, y_end); y_start--; y_end--; };

                for (int y = y_start; y_flip ? y > y_end : y < y_end;) {
                    int y_off = y_flip ? ((spsize-1) - (y % spsize)) : (y % spsize);

                    //if (TEST_REG(PPU_LCDC, LCDC_SPSIZE)) {
                    //    if (y > 8) {
                    //        t |= 0x1;
                    //    } else {
                    //        t &= 0xfe;
                    //    }
                    //}
                    
                    u8 l = spr_bank->at((t * 16) + (y_off * 2)),
                       h = spr_bank->at((t * 16) + (y_off * 2) + 1);

                    for (int x = x_start; x_flip ? x > x_end : x < x_end;) {
                        int px = x_flip ? (x % 8) : (7 - (x % 8));

                        u8 pal_offset = ((l >> px) & 1) | (((h >> px) & 1) << 1);

                        if (pal_offset) {
                            u8 color = 0;
                            u32 out = 0;

                            if (settings::cgb_mode) {
                                color = pal_offset;
                                u8 off = ((attr & SPATTR_PALN) << 3) + (color << 1);
                                u16 p = cgb_spr_palette[off] | (cgb_spr_palette[off+1] << 8);
                                //_log(debug, "p=%04x", p);
                                u8 r = (p & 0x1f) << 3,
                                   g = (((p >> 0x5) & 0x1f) << 3),
                                   b = (((p >> 0xa) & 0x1f) << 3);

                                out = lgw::rgb(r, g, b);
                            } else {
                                color = ((attr & SPATTR_PALL ? r[PPU_OBP1] : r[PPU_OBP0]) >> (pal_offset * 2)) & 0x3;
                                out = color_palette[color];
                            }

                            if ((x + (bx-8) < PPU_WIDTH) && (y + (by-16) < PPU_HEIGHT))
                                frame.draw(x + (bx-8), y + (by-16), out);
                        }

                        if (x_flip) { x--; } else { x++; }
                    }
                    if (y_flip) { y--; } else { y++; }
                }
            }
        }

        u32 read(u16 addr, size_t size) {
            // Handle JOYP reads
            if (addr == 0xff00) { return joypad::read(); }
            if (addr == MMIO_VBK) { return 0xfe | (current_bank_idx & 0x1); }

            if (addr == 0xff69) {
                return utility::default_mb_read(cgb_bg_palette.data(), cgb_bg_palette_idx >= 0x40 ? 0x40 : cgb_bg_palette_idx, size);
            }
            if (addr == 0xff6b) {
                return utility::default_mb_read(cgb_spr_palette.data(), cgb_spr_palette_idx >= 0x40 ? 0x40 : cgb_spr_palette_idx, size);
            }

            if (addr >= PPU_R_BEGIN && addr <= PPU_R_END) {
                return utility::default_mb_read(r.data(), addr, size, PPU_R_BEGIN);
            }

            if (settings::inaccessible_vram_emulation_enabled) { if (vram_disabled()) return 0xff; }

            if (addr >= VRAM_BEGIN && addr <= VRAM_END) {
                return utility::default_mb_read(current_bank->data(), addr, size, VRAM_BEGIN);
            }

            if (settings::inaccessible_vram_emulation_enabled) { if (oam_disabled()) return 0xff; }

            if (addr >= OAM_BEGIN && addr <= OAM_END) {
                return utility::default_mb_read(oam.data(), addr, size, OAM_BEGIN);
            }

            return 0;
        }

        void write(u16 addr, u16 value, size_t size) {
#ifdef GEEBLY_NES_MODE
            if ((addr == 0xff42) || (addr == 0xff43)) return;
#endif

            if (addr == 0xff00) { joypad::write(value); }

            if (addr >= PPU_R_BEGIN && addr <= PPU_R_END) {
                utility::default_mb_write(r.data(), addr, value, size, PPU_R_BEGIN);
                return;
            }

            if (addr == MMIO_VBK) {
                current_bank_idx = value & 1;
                current_bank = &vram[current_bank_idx];
                return;
            }
            
            if (addr == 0xff68) {
                cgb_bg_palette_idx = value & 0x3f;
                bg_auto_inc = value & 0x80;
                return;
            }

            if (addr == 0xff69) {
                utility::default_mb_write(cgb_bg_palette.data(), cgb_bg_palette_idx >= 0x40 ? 0x40 : cgb_bg_palette_idx, value, size, 0);
                if (bg_auto_inc) cgb_bg_palette_idx++;
                return;
            }

            if (addr == 0xff6a) {
                cgb_spr_palette_idx = value & 0x3f;
                spr_auto_inc = value & 0x80;
                return;
            }

            if (addr == 0xff6b) {
                utility::default_mb_write(cgb_spr_palette.data(), cgb_spr_palette_idx >= 0x40 ? 0x40 : cgb_spr_palette_idx, value, size, 0);
                if (spr_auto_inc) cgb_spr_palette_idx++;
                return;
            }

            if (settings::inaccessible_vram_emulation_enabled) { if (vram_disabled()) return; }

            if (addr >= VRAM_BEGIN && addr <= VRAM_END) {
                utility::default_mb_write(current_bank->data(), addr, value, size, VRAM_BEGIN);
                return;
            }

            if (settings::inaccessible_vram_emulation_enabled) { if (oam_disabled()) return; }

            if (addr >= OAM_BEGIN && addr <= OAM_END) {
                utility::default_mb_write(oam.data(), addr, value, size, OAM_BEGIN);
                return;
            }
        }

        u8& ref(u16 addr) {
            if (addr >= PPU_R_BEGIN && addr <= PPU_R_END) { return r[addr-PPU_R_BEGIN]; }

            if (addr == MMIO_VBK) { return current_bank_idx; }

            if (settings::inaccessible_vram_emulation_enabled) { if (vram_disabled()) return dummy; }
            
            if (addr >= VRAM_BEGIN && addr <= VRAM_END) { return (*current_bank)[addr-VRAM_BEGIN]; }

            if (settings::inaccessible_vram_emulation_enabled) { if (oam_disabled()) return dummy; }

            if (addr >= OAM_BEGIN && addr <= OAM_END) { return oam[addr-OAM_BEGIN]; }

            return dummy;
        }

        void close() {
            SDL_DestroyWindow(sdl::window);
            SDL_DestroyRenderer(sdl::renderer);
            SDL_Quit();
        }

        // Clean this up
        bool vbl_pure_fired = false,
             lyc_stat_fired = false,
             oam_stat_fired = false,
             vbl_stat_fired = false,
             hbl_stat_fired = false;
        
        void cycle() {
            if (stopped) r[PPU_LCDC] &= ~LCDC_SWITCH;

            current_bank = &vram[current_bank_idx];
            
            switch (r[PPU_STAT] & 3) {
                // HBlank mode
                case 0: {
                    if (clk >= 204) {
                        if (TEST_REG(PPU_STAT, STAT_MODE00)) {
                            if (!hbl_stat_fired) {
                                ic::ref(0xff0f) |= IRQ_STAT;
                                hbl_stat_fired = true;
                            }
                        }

                        clk = 0;
                        r[PPU_LY]++;

                        if (r[PPU_LY] == r[PPU_LYC]) {
                            if (TEST_REG(PPU_STAT, STAT_LYCNSD)) {
                                if (!lyc_stat_fired) {
                                    ic::ref(0xff0f) |= IRQ_STAT;
                                    lyc_stat_fired = true;
                                }
                            }
                            r[PPU_STAT] |= 0x4;
                        } else {
                            if (TEST_REG(PPU_STAT, STAT_LYCNSD)) {
                                lyc_stat_fired = false;
                            }
                            r[PPU_STAT] &= ~0x4;
                        }

                        if (r[PPU_LY] == PPU_HEIGHT) {
                            // Switch state to Vblank
                            r[PPU_STAT] &= 0xfc;
                            r[PPU_STAT] |= 1;
                            
                            // Draw frame
                            SDL_RenderClear(sdl::renderer);

                            if (TEST_REG(PPU_LCDC, LCDC_SWITCH)) {
                                if (TEST_REG(PPU_LCDC, LCDC_SPDISP)) render_sprites();

                                SDL_UpdateTexture(
                                    sdl::texture,
                                    NULL,
                                    frame.get_buffer(),
                                    PPU_WIDTH * sizeof(uint32_t)
                                );
                                
                                SDL_RenderCopy(sdl::renderer, sdl::texture, NULL, NULL);
                                
                                frame.clear();
                            }

                            update_window();

                            SDL_RenderPresent(sdl::renderer);
                        } else {
                            // Switch to OAM read
                            r[PPU_STAT] &= 0xfc;
                            r[PPU_STAT] |= 2;
                        }
                    }
                } break;

                // VBlank mode
                case 1: {
                    // Trigger Vblank interrupt
                    if (clk >= 456) {
                        // Fire interrupt
                        if (!vbl_pure_fired) {
                            ic::ref(0xff0f) |= IRQ_VBL;
                            vbl_pure_fired = true;
                        }

                        if (TEST_REG(PPU_STAT, STAT_MODE01)) {
                            if (!vbl_stat_fired) {
                                ic::ref(0xff0f) |= IRQ_STAT;
                                vbl_stat_fired = true;
                            }
                        }

                        clk = 0;
                        r[PPU_LY]++;

                        if (r[PPU_LY] >= 153) {
                            r[PPU_STAT] &= 0xfc;
                            r[PPU_STAT] |= 2;
                            r[PPU_LY] = 0;
                        }
                    }
                } break;

                // OAM read period
                case 2: {
                    vbl_pure_fired = false;
                    vbl_stat_fired = false;
                    hbl_stat_fired = false;

                    if (TEST_REG(PPU_STAT, STAT_MODE02)) {
                        if (!oam_stat_fired) {
                            ic::ref(0xff0f) |= IRQ_STAT;
                            oam_stat_fired = true;
                        }
                    }

                    if (clk >= 80) {
                        clk = 0;
                        r[PPU_STAT] &= 0xfc;
                        r[PPU_STAT] |= 3;
                    }
                } break;

                // VRAM read period
                case 3: {
                    oam_stat_fired = false;

                    if (clk >= 172) {
                        clk = 0;
                        r[PPU_STAT] &= 0xfc;

                        if (TEST_REG(PPU_LCDC, LCDC_SWITCH)) {
                            render_scanline();
                        }
                    }
                }
            }
            
            clk += clock::get() >> 2;
        }
    }
}

#undef TEST_REG