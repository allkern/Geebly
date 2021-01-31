#pragma once

#include <iostream>

#include "../../lgw/framebuffer.hpp"
#include "../../aliases.hpp"
#include "../../global.hpp"
#include "../joypad.hpp"
#include "../clock.hpp"
#include "../ic.hpp"
#include "memory.hpp"

#include <cstdlib>
#include <array>

#define PPU_WIDTH  160
#define PPU_HEIGHT 144

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

namespace gameboy {
    namespace ppu {
        sf::RenderWindow window(sf::VideoMode(PPU_WIDTH, PPU_HEIGHT), "Geebly");
        lgw::framebuffer frame;

        u32 color_palette[] = {
            0xfffffffful,
            0xaaaaaafful,
            0x555555fful,
            0x000000fful
        };

        //u32 color_palette[] = {
        //    0xe0f8d0fful,
        //    0x88c070fful,
        //    0x346856fful,
        //    0x081820fful
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

        // Fix JOYP emulation
        inline void update_window() {
            sf::Event event;
            if (window.pollEvent(event)) {
                switch (event.type) {
                    case sf::Event::Closed: { window.close(); window_closed = true; return; } break;
                    case sf::Event::KeyPressed: { joypad::keydown(event.key.code); } break;
                    case sf::Event::KeyReleased: { joypad::keyup(event.key.code); } break;
                    default: break;
                }
            }
        }

        void init(int scale) {
            frame.init(PPU_WIDTH, PPU_HEIGHT, sf::Color(color_palette[3]));

            //window.setPosition(sf::Vector2i(100, 100));
            
            window.setFramerateLimit(60);

            // Holding a key would act like a turbo button otherwise
            window.setKeyRepeatEnabled(false);

            // Fill VRAM and OAM with random values 
            srand(time(NULL));

            for (auto& b : vram[0]) { b = rand() % 0xff; }
            for (auto& b : vram[1]) { b = rand() % 0xff; }
            for (auto& b : oam) { b = rand() % 0xff; }

            window.setSize(sf::Vector2u(PPU_WIDTH*scale, PPU_HEIGHT*scale));

            // Start PPU on mode 2 (OAM read)
            r[PPU_STAT] = 0x84;

            if (settings::skip_bootrom) {
                r[PPU_LCDC] = 0x91;
                r[PPU_STAT] = 0x85;
                r[PPU_BGP] = 0xfc;
            }

            window.clear(sf::Color(color_palette[3]));
            window.display();
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

        // Maybe should probably implement this as a state machine later on?
        void render_scanline() {
            bool window = TEST_REG(PPU_LCDC, LCDC_WNDSWI) && (r[PPU_LY] >= r[PPU_WY]);

            u8 sw_mask  = window ? LCDC_WNDSWI  : LCDC_BGWSWI,
               scrolly  = window ? PPU_WY       : PPU_SCY,
               scrollx  = window ? PPU_WX       : PPU_SCX,
               tilemap  = window ? LCDC_WNDTMS  : LCDC_BGWTMS;

            if (TEST_REG(PPU_LCDC, sw_mask)) {
                u8 y = r[PPU_LY] + (window ? (-r[scrolly]) : r[scrolly]);

                // Get tilemap and tileset offsets based on bits 3 and 4 of LCDC
                u16 tilemap_offset = (TEST_REG(PPU_LCDC, tilemap) ? 0x1c00 : 0x1800),
                    tileset_offset = (TEST_REG(PPU_LCDC, LCDC_BGWTSS) ? 0x0 : 0x800);
                
                #define TILE_OFFSET tilemap_offset + ((y >> 3) * 32) + ((r[scrollx] - (window ? 7 : 0)) >> 3)

                // Signed mode requires a slightly different offset calculation
                #define CHAR_OFFSET \
                    (TEST_REG(PPU_LCDC, LCDC_BGWTSS)) ? \
                        (tile * 16) + ((y % 8) * 2) : \
                        0x1000 + (((s8)tile * 16) + ((y % 8) * 2));
                
                u8 tile = vram[0][TILE_OFFSET];

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
                for (int x = 0; x < PPU_WIDTH; x++) {
                    if (TEST_REG(PPU_LCDC, sw_mask)) {
                        int px = 7 - ((x + ((r[scrollx] - (window ? 7 : 0)) % 8)) % 8);

                        u8 pal_offset = ((l >> px) & 1) | (((h >> px) & 1) << 1);

                        int color = 0;
                        u32 out = 0;
                    
                        if (settings::cgb_mode) {
                            color = pal_offset;
                            u8 off = (bg_attr.bgp << 3) + (color << 1);
                            u16 p = cgb_palette[off] | (cgb_palette[off+1] << 8);
                            u8 r = (p & 0x1f) << 3,
                               g = (((p >> 0x5) & 0x1f) << 3),
                               b = (((p >> 0xa) & 0x1f) << 3);
                            out = ((u32)r << 0x18) | ((u32)g << 0x10) | ((u32)b << 0x8) | 0xff;
                        } else {
                            color = (r[PPU_BGP] >> (pal_offset * 2)) & 0x3;
                            out = color_palette[color];
                        }

                        frame.draw(x, r[PPU_LY], sf::Color(out));

                        // Recalculate offsets and settings when reaching tile boundaries
                        if (px == 0) {
                            tile = vram[0][++TILE_OFFSET];
                            char_base = CHAR_OFFSET;

                            if (settings::cgb_mode) {
                                u8 bg_attr_byte = vram[1][TILE_OFFSET];

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

            u32 draw_color = 0;

            int spsize = TEST_REG(PPU_LCDC, LCDC_SPSIZE) ? 16 : 8;

            while (addr <= 0x9f) {
                int y_start = 0,
                    y_end = spsize,
                    x_start = 0,
                    x_end = 8;
                
                u8 by = oam[addr++],
                   bx = oam[addr++],
                   t = oam[addr++] & (TEST_REG(PPU_LCDC, LCDC_SPSIZE) ? 0xfe : 0xff),
                   attr = oam[addr++];

                bool x_flip = (attr & SPATTR_XFLP),
                     y_flip = (attr & SPATTR_YFLP);

                if (x_flip) { std::swap(x_start, x_end); x_start--; x_end--; };
                if (y_flip) { std::swap(y_start, y_end); y_start--; y_end--; };

                for (int y = y_start; y_flip ? y > y_end : y < y_end;) {
                    int y_off = y_flip ? ((spsize-1) - (y % spsize)) : (y % spsize);

                    u8 l = vram[0][(t * 16) + (y_off * 2)],
                       h = vram[0][(t * 16) + (y_off * 2) + 1];

                    for (int x = x_start; x_flip ? x > x_end : x < x_end;) {
                        int px = x_flip ? (x % 8) : (7 - (x % 8));

                        u8 pal_offset = ((l >> px) & 1) | (((h >> px) & 1) << 1);

                        if (pal_offset) {
                            u8 color = ((attr & SPATTR_PALL ? r[PPU_OBP1] : r[PPU_OBP0]) >> (pal_offset * 2)) & 0x3;

                            if ((x + (bx-8) < PPU_WIDTH) && (y + (by-16) < PPU_HEIGHT))
                                frame.draw(x + (bx-8), y + (by-16), sf::Color(color_palette[color]));
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
            if (addr == 0xff4f) { return current_bank_idx; }
            if (addr == 0xff69) {
                return utility::default_mb_read(cgb_palette.data(), cgb_palette_idx >= 0x40 ? 0x40 : cgb_palette_idx, size);
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
            if (addr == 0xff00) { joypad::write(value); }

            if (addr >= PPU_R_BEGIN && addr <= PPU_R_END) {
                utility::default_mb_write(r.data(), addr, value, size, PPU_R_BEGIN);
                return;
            }

            if (addr == 0xff4f) {
                current_bank_idx = value & 1;
                current_bank = &vram[current_bank_idx];
                return;
            }

            if (addr == 0xff68) {
                cgb_palette_idx = value & 0x3f;
                auto_inc = value & 0x80;
                return;
            }

            if (addr == 0xff69) {
                utility::default_mb_write(cgb_palette.data(), cgb_palette_idx >= 0x40 ? 0x40 : cgb_palette_idx, value, size, 0);
                if (auto_inc) cgb_palette_idx++;
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

            if (settings::inaccessible_vram_emulation_enabled) { if (vram_disabled()) return dummy; }
            
            if (addr >= VRAM_BEGIN && addr <= VRAM_END) { return (*current_bank)[addr-VRAM_BEGIN]; }

            if (settings::inaccessible_vram_emulation_enabled) { if (oam_disabled()) return dummy; }

            if (addr >= OAM_BEGIN && addr <= OAM_END) { return oam[addr-OAM_BEGIN]; }

            return dummy;
        }

        // Clean this up
        bool fired = false;
        
        void cycle() {
            switch (r[PPU_STAT] & 3) {
                // HBlank mode
                case 0: {
                    if (clk >= 204) {
                        clk = 0;
                        r[PPU_LY]++;
                        if (r[PPU_LY] == r[PPU_LYC]) {
                            r[PPU_STAT] |= 0x4;
                        } else {
                            r[PPU_STAT] &= ~0x4;
                        }

                        if (r[PPU_LY] == 0x94) {
                            // Switch state to Vblank
                            r[PPU_STAT] &= 0xfc;
                            r[PPU_STAT] |= 1;
                            
                            // Draw frame
                            if (TEST_REG(PPU_LCDC, LCDC_SWITCH)) {
                                if (TEST_REG(PPU_LCDC, LCDC_SPDISP)) render_sprites();

                                //window.clear(sf::Color::Black);

                                window.draw(*frame.get_drawable());
                                frame.clear();
                            }

                            update_window();

                            window.display();
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
                        if (!fired) {
                            ic::ref(0xff0f) |= 0x1;
                            fired = true;
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
                    fired = false;
                    if (clk >= 80) {
                        clk = 0;
                        r[PPU_STAT] &= 0xfc;
                        r[PPU_STAT] |= 3;
                    }
                } break;

                // VRAM read period
                case 3: {
                    if (clk >= 172) {
                        clk = 0;
                        r[PPU_STAT] &= 0xfc;

                        if (TEST_REG(PPU_LCDC, LCDC_SWITCH)) {
                            render_scanline();
                        }
                    }
                }
            }
            
            clk += clock::get();
        }
    }
}