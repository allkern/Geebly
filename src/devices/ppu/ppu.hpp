#pragma once

#include <iostream>

#include "../../lgw/framebuffer.hpp"
#include "../../aliases.hpp"
#include "../io.hpp"

#include "memory.hpp"

#include <sys/unistd.h>
#include <cstdlib>

#include <array>

#define PPU_WIDTH  160
#define PPU_HEIGHT 144

namespace gameboy {
    namespace ppu {
        sf::RenderWindow window(sf::VideoMode(PPU_WIDTH, PPU_HEIGHT), "Geebly 2.0");

        lgw::framebuffer frame;

        u8 rows[2] = { 0xf, 0xf };
        u8 column = 0;

        int clk;

        u8* last_cpu_time = nullptr;

        //u32 color_palette[] = {
        //    0xfffffffful,
        //    0xaaaaaafful,
        //    0x555555fful,
        //    0x000000fful
        //};

        //u32 color_palette[] = {
        //    0xe0f8d0fful,
        //    0x88c070fful,
        //    0x346856fful,
        //    0x081820fful
        //};

        u32 color_palette[] = {
            0xe8fcccfful,
            0xacd490fful,
            0x548c70fful,
            0x081820fful
        };

        // PPU register indexes
        #define PPU_LCDC    0x0
        #define PPU_STAT    0x1
        #define PPU_SCY     0x2
        #define PPU_SCX     0x3
        #define PPU_LY      0x4
        #define PPU_LYC     0x5

        // DMA controller not handled by the PPU
        // #define PPU_DMA     0x6
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

        // JOYP masks
        #define JOYP_BUTTON 0b00100000
        #define JOYP_DIRECT 0b00010000
        #define JOYP_START  0b00001000
        #define JOYP_SELECT 0b00000100
        #define JOYP_B      0b00000010
        #define JOYP_A      0b00000001
        #define JOYP_DOWN   0b00001000
        #define JOYP_UP     0b00000100
        #define JOYP_LEFT   0b00000010
        #define JOYP_RIGHT  0b00000001

        // Macro to test register bits
        #define TEST_REG(reg, mask) (r[reg] & mask)

        void init(int scale, u8& cpu_last_cycles_register) {
            frame.init(PPU_WIDTH, PPU_HEIGHT, sf::Color(color_palette[3]));
            
            window.setFramerateLimit(60);

            // Holding a key would act like a turbo button otherwise
            window.setKeyRepeatEnabled(false);

            // Fill VRAM and OAM with random values 
            srand(time(NULL));

            for (auto& b : vram) { b = rand() % 0xff; }
            for (auto& b : oam) { b = rand() % 0xff; }

            window.setSize(sf::Vector2u(PPU_WIDTH*scale, PPU_HEIGHT*scale));

            // Start PPU on mode 2 (OAM read)
            if (skip_bootrom) {
                r[PPU_STAT] = 0x85;
            }

            window.clear(sf::Color(color_palette[3]));
            window.display();

            last_cpu_time = &cpu_last_cycles_register;
        }

        inline bool vram_disabled() {
            // Disable VRAM and OAM on modes 2 and 3, and when LCDC bit 7 is low
            return (r[PPU_STAT] & 3) >= 2 && TEST_REG(PPU_LCDC, LCDC_SWITCH);
        }

        // Maybe should probably implement this as a state machine later on?
        void render_scanline() {
            if (TEST_REG(PPU_LCDC, LCDC_BGWSWI)) {
                u8 y = ((r[PPU_LY]) + r[PPU_SCY]);

                // Get tilemap and tileset offsets based on bits 3 and 4 of LCDC
                u16 tilemap_offset = (TEST_REG(PPU_LCDC, LCDC_BGWTMS) ? 0x1c00 : 0x1800),
                    tileset_offset = (TEST_REG(PPU_LCDC, LCDC_BGWTSS) ? 0x0 : 0x800);
                
                #define TILE_OFFSET tilemap_offset + ((y >> 3) * 32)

                // Signed mode requires a slightly different offset calculation
                #define CHAR_OFFSET \
                    (TEST_REG(PPU_LCDC, LCDC_BGWTSS)) ? \
                        (tile * 16) + ((y % 8) * 2) : \
                        0x1000 + (((s8)tile * 16) + ((y % 8) * 2));
                
                u8 tile = vram[TILE_OFFSET];

                u16 char_base = CHAR_OFFSET;
                u8 l = vram[char_base], h = vram[char_base + 1];

                // Start rendering the whole scanline
                for (int x = 0; x < 160; x++) {
                    int px = 7 - (x % 8);

                    // Cleanup these formulas?
                    u8 pal_offset = ((l & (1 << px)) >> px) | (((h & (1 << px)) >> px) << 1);

                    int color = (r[PPU_BGP] & (3 << (pal_offset * 2))) >> (pal_offset * 2);

                    frame.draw(x, r[PPU_LY], sf::Color(color_palette[color]));

                    // Recalculate offsets when reaching the tile boundary
                    if (px == 0) {
                        tile = vram[++TILE_OFFSET];
                        char_base = CHAR_OFFSET;
                        l = vram[char_base];
                        h = vram[char_base + 1];
                    }
                }
            }
        }

        // Clean this up
        void draw_sprites() {
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
                   t = oam[addr++],
                   attr = oam[addr++];

                if (attr & SPATTR_XFLP) std::swap(x_start, x_end);
                if (attr & SPATTR_YFLP) std::swap(y_start, y_end);

                for (int y = y_start; y < y_end;) {
                    u8 l = vram[(t * 16) + ((y % spsize) * 2)],
                       h = vram[(t * 16) + ((y % spsize) * 2) + 1];

                    for (int x = x_start; x < x_end;) {
                        int px = 7 - (x % 8);

                        u8 pal_offset = ((l & (1 << px)) >> px) | (((h & (1 << px)) >> px) << 1);

                        int color = ((attr & SPATTR_PALL ? r[PPU_OBP0] : r[PPU_OBP1]) & (3 << (pal_offset * 2))) >> (pal_offset * 2);

                        if (color != 0) frame.draw(x + (bx-8), y + (by-16), sf::Color(color_palette[color]));

                        if (attr & SPATTR_XFLP) { x--; } else { x++; }
                    }

                    if (attr & SPATTR_YFLP) { y--; } else { y++; }
                }
            }
        }

        u32 read(u16 addr, size_t size) {
            // Handle JOYP reads
            if (addr == 0xff00) {
                if (column == 0x10) {
                    return rows[0] & 0xf;
                }

                if (column == 0x20) {
                    return rows[1] & 0xf;
                }

                return 0xff;
            }

            if (addr >= PPU_R_BEGIN && addr <= PPU_R_END) {
                return utility::default_mb_read(r.data(), addr, size, PPU_R_BEGIN);
            }

            if (inaccessible_vram_emulation_enabled) { if (vram_disabled()) return 0xff; }

            if (addr >= VRAM_BEGIN && addr <= VRAM_END) {
                return utility::default_mb_read(vram.data(), addr, size, VRAM_BEGIN);
            }

            if (addr >= OAM_BEGIN && addr <= OAM_END) {
                return utility::default_mb_read(oam.data(), addr, size, OAM_BEGIN);
            }

            return 0;
        }

        void write(u16 addr, u16 value, size_t size) {
            if (addr == 0xff00) {
                column = value & 0x30;
                return;
            }

            if (addr >= PPU_R_BEGIN && addr <= PPU_R_END) {
                utility::default_mb_write(r.data(), addr, value, size, PPU_R_BEGIN);
                return;
            }

            if (inaccessible_vram_emulation_enabled) { if (vram_disabled()) return; }

            if (addr >= VRAM_BEGIN && addr <= VRAM_END) {
                utility::default_mb_write(vram.data(), addr, value, size, VRAM_BEGIN);
                return;
            }

            if (addr >= OAM_BEGIN && addr <= OAM_END) {
                utility::default_mb_write(oam.data(), addr, value, size, OAM_BEGIN);
                return;
            }
        }

        u8& ref(u16 addr) {
            if (addr >= PPU_R_BEGIN && addr <= PPU_R_END) { return vram[addr-PPU_R_BEGIN]; }

            if (inaccessible_vram_emulation_enabled) { if (vram_disabled()) return dummy; }
            
            if (addr >= VRAM_BEGIN && addr <= VRAM_END) { return vram[addr-VRAM_BEGIN]; }
            if (addr >= OAM_BEGIN && addr <= OAM_END) { return vram[addr-OAM_BEGIN]; }

            return dummy;
        }

        // Fix JOYP emulation
        inline void update_window() {
            sf::Event event;
            if (window.pollEvent(event)) {
                switch (event.type) {
                    case sf::Event::Closed: { window.close(); window_closed = true; } break;
                    case sf::Event::KeyPressed: {
                        switch (event.key.code) {
                            case sf::Keyboard::Enter: { rows[0] &= ~(JOYP_START ); } break;
                            case sf::Keyboard::Q    : { rows[0] &= ~(JOYP_SELECT); } break;
                            case sf::Keyboard::A    : { rows[0] &= ~(JOYP_A     ); } break;
                            case sf::Keyboard::S    : { rows[0] &= ~(JOYP_B     ); } break;
                            case sf::Keyboard::Up   : { rows[1] &= ~(JOYP_UP    ); } break;
                            case sf::Keyboard::Down : { rows[1] &= ~(JOYP_DOWN  ); } break;
                            case sf::Keyboard::Left : { rows[1] &= ~(JOYP_LEFT  ); } break;
                            case sf::Keyboard::Right: { rows[1] &= ~(JOYP_RIGHT ); } break;
                        }
                    } break;
                    case sf::Event::KeyReleased: {
                        switch (event.key.code) {
                            case sf::Keyboard::Enter: { rows[0] |= (JOYP_START ); } break;
                            case sf::Keyboard::Q    : { rows[0] |= (JOYP_SELECT); } break;
                            case sf::Keyboard::A    : { rows[0] |= (JOYP_A     ); } break;
                            case sf::Keyboard::S    : { rows[0] |= (JOYP_B     ); } break;
                            case sf::Keyboard::Up   : { rows[1] |= (JOYP_UP    ); } break;
                            case sf::Keyboard::Down : { rows[1] |= (JOYP_DOWN  ); } break;
                            case sf::Keyboard::Left : { rows[1] |= (JOYP_LEFT  ); } break;
                            case sf::Keyboard::Right: { rows[1] |= (JOYP_RIGHT ); } break;
                        }
                    } break;
                }
            }
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

                        if (r[PPU_LY] == 0x94) {
                            // Switch state to Vblank
                            r[PPU_STAT] &= 0xfc;
                            r[PPU_STAT] |= 1;
                            
                            // Draw frame
                            if (TEST_REG(PPU_LCDC, LCDC_SWITCH)) {
                                update_window();

                                if (TEST_REG(PPU_LCDC, LCDC_SPDISP)) draw_sprites();

                                window.draw(*frame.get_drawable());
                            }
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
                        if (!fired) {
                            u8& ia = io::ref(0xff0f);
                            ia |= 1;
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
            
            clk += (*last_cpu_time);
        }
    }
}