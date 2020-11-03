#pragma once

#include "../lgw/framebuffer.hpp"
#include "../aliases.hpp"

#include <sys/unistd.h>

#include <array>

#define PPU_BEGIN   0x8000
#define PPU_END     0x9fff
#define VRAM_BEGIN  PPU_BEGIN
#define VRAM_END    PPU_END
#define CRAM_BEGIN  PPU_BEGIN
#define CRAM_END    0x97ff
#define TM0_BEGIN   0x9800
#define TM0_END     0x9bff
#define TM1_BEGIN   0x9c00
#define TM1_END     PPU_END
#define OAM_BEGIN   0xfe00
#define OAM_END     0xfe9f
#define PPU_R_BEGIN 0xff40
#define PPU_R_END   0xff4b

namespace gameboy {
    //template <class T> std::string bin(T v) {
    //    std::string s;
    //    for (int b = (sizeof(T) * 8) - 1; b >= 0; b--) {
    //        s += (v & (1 << b)) ? '1' : '0';
    //    }
    //    return s;
    //}

    namespace ppu {
        typedef std::array <u8, 0x2000> vram_t;
        typedef std::array <u8, 0xa0> oam_t;
        typedef std::array <u8, 0xc> ppu_registers_t;

        u8 dummy = 0;

        sf::RenderWindow window(sf::VideoMode(160*2, 144*2), "Geebly 1.0a");
        lgw::framebuffer frame;

        vram_t vram = { 0 }; // 0x1fff size
        oam_t oam  = { 0 }; // 0x9f size
        ppu_registers_t r = { 0 };

        struct sprite {
            u8 x, y, t;
            bool priority, x_flip, y_flip, pal;
        };

        int mode, clk;

        u8* last_cpu_time = nullptr;

        bool lcd_enabled = true;
        bool window_tilemap; // 0: 9800-9BFF, 1: 9C00-9FFF
        bool window_enabled;
        bool background_tileset = true; // 0: 8800-97FF, 1: 8000-8FFF
        bool sprite_size; // 0: 8x8, 1: 8x16
        bool sprites_enabled;
        bool background_enabled = true;

        u32 color_palette[] = {
            0xfffffffful,
            0xaaaaaafful,
            0x555555fful,
            0x000000fful
        };

        u8 scroll_x = 0, scroll_y = 0;

        #define PPU_LCDC    0
        #define PPU_STAT    1
        #define PPU_SCY     2
        #define PPU_SCX     3
        #define PPU_LY      4
        #define PPU_LYC     5
        #define PPU_DMA     6
        #define PPU_BGP     7
        #define PPU_OBP0    8
        #define PPU_OBP1    9
        #define PPU_WY      10
        #define PPU_WX      11

        #define LCDC_SWITCH 0b10000000 // lcd_enabled
        #define LCDC_WNDTMS 0b01000000 // window_tilemap
        #define LCDC_WNDSWI 0b00100000 // window_enabled
        #define LCDC_BGWTSS 0b00010000 // background_tileset
        #define LCDC_BGWTMS 0b00001000 // background_tilemap (0: 9800-9BFF, 1: 9C00-9FFF)
        #define LCDC_SPSIZE 0b00000100 // sprite_size
        #define LCDC_SPDISP 0b00000010 // sprites_enabled
        #define LCDC_BGWSWI 0b00000001 // background_enabled

        #define STAT_LYCNSD 0b01000000
        #define STAT_MODE02 0b00100000
        #define STAT_MODE01 0b00010000
        #define STAT_MODE00 0b00001000
        #define STAT_CDFLAG 0b00000100
        #define STAT_CRMODE 0b00000011

        #define TEST_REG(reg, mask) (r[reg] & mask)

        void init(u8& cpu_last_cycles_register) {
            frame.init(160, 144);

            vram.fill(0);
            oam.fill(0);
            r[PPU_LCDC] = 0x91;

            last_cpu_time = &cpu_last_cycles_register;
        }

        void render_scanline() {
            if (TEST_REG(PPU_LCDC, LCDC_BGWSWI)) {
                int y = r[PPU_LY] - 1;
                //// The current pixel position in either the x or y direction
                //// divided by 8 gives the location in the background tilemap
                //int tile_x = scroll_x >> 3,
                //    tile_y = ((r[PPU_LY] + scroll_y) & 0xff) >> 3;
//
                //std::cout << tile_x << " " << tile_y << std::endl;
                //return;
//
                //// The 3 sm of the leftmost pixel of the current
                //// scanline give the pixel coordinates in the first tile
                //int pixel_x = scroll_x & 7;
                //int pixel_y = (r[PPU_LY] + scroll_y) & 7;

                // The offset address in VRAM for either tilemap 1 or 0
                int tilemap_offset = (TEST_REG(PPU_LCDC, LCDC_BGWTMS) ? 0x1c00 : 0x1800);
                
                #define TILE_OFFSET tilemap_offset + ((y >> 3) * 32)
                #define CHAR_OFFSET (tile * 16) + ((y % 8) * 2)
                
                int tile = vram[TILE_OFFSET];

                int char_base = CHAR_OFFSET;
                int l = vram[char_base],
                    h = vram[char_base + 1];

                // Start rendering the whole scanline
                for (int x = 0; x < 160; x++) {
                    int px = 7 - (x % 8);

                    u8 pal_offset = ((l & (1 << px)) >> px) | (((h & (1 << px)) >> px) << 1);

                    int color = (r[PPU_BGP] & (3 << (pal_offset * 2))) >> (pal_offset * 2);

                    frame.draw(x, y, sf::Color(color_palette[color]));

                    if (px == 0) {
                        tile = vram[++TILE_OFFSET];
                        char_base = CHAR_OFFSET;
                        l = vram[char_base];
                        h = vram[char_base + 1];
                    }
                }
            }
        }

        u32 read(u16 addr, size_t size) {
            u32 d = 0;
            if (addr >= PPU_R_BEGIN && addr <= PPU_R_END) {
                while (size) {
                    d |= r[(addr+(size-1))-PPU_R_BEGIN] << (((size--)-1)*8);
                }
                return d;
            }
            if (r[PPU_STAT] & 3 == 3) return 0xff;
            if (addr >= VRAM_BEGIN && addr <= VRAM_END) { 
                while (size) {
                    d |= vram[(addr+(size-1))-VRAM_BEGIN] << (((size--)-1)*8);
                }
                return d;
            }

            if (r[PPU_STAT] & 3 == 2) return 0xff;
            if (addr >= OAM_BEGIN && addr <= OAM_END) {
                while (size) {
                    d |= oam[(addr+(size-1))-OAM_BEGIN] << (((size--)-1)*8);
                }
                return d;
            }

            return 0;
        }

        void write(u16 addr, u16 value, size_t size) {
            int s = 0;
            if (addr >= PPU_R_BEGIN && addr <= PPU_R_END) {
                while (size--) {
                    vram[(addr++)-PPU_R_BEGIN] = value & (0xff << (s++)*8);
                }
            }
            if (r[PPU_STAT] & 3 == 3) return;
            if (addr >= VRAM_BEGIN && addr <= VRAM_END) {
                while (size--) {
                    vram[(addr++)-VRAM_BEGIN] = value & (0xff << (s++)*8);
                }
            }
            if (r[PPU_STAT] & 3 == 2) return;
            if (addr >= OAM_BEGIN && addr <= OAM_END) {
                while (size--) {
                    oam[(addr++)-OAM_BEGIN] = value & (0xff << (s++)*8);
                }
            }
        }

        u8& ref(u16 addr) {
            int s = 0;
            if (addr >= PPU_R_BEGIN && addr <= PPU_R_END) { return vram[addr-PPU_R_BEGIN]; }
            if (r[PPU_STAT] & 3 == 3) return dummy;
            if (addr >= VRAM_BEGIN && addr <= VRAM_END) { return vram[addr-VRAM_BEGIN]; }
            if (r[PPU_STAT] & 3 == 2) return dummy;
            if (addr >= OAM_BEGIN && addr <= OAM_END) { return vram[addr-OAM_BEGIN]; }

            return dummy;
        }
        
        void cycle() {
            clk += *last_cpu_time;

            sf::Event event;
            while (window.pollEvent(event)) {
                switch (event.type) {
                    case sf::Event::Closed: { window.close(); } break;
                }
            }

            switch (r[PPU_STAT] & 3) {
                // HBlank mode
                case 0: {
                    if (clk >= 204) {
                        clk = 0;
                        r[PPU_LY]++;

                        if (r[PPU_LY] == 143) {
                            r[PPU_STAT] &= 0xfc;
                            r[PPU_STAT] |= 1;

                            auto f = frame.get_drawable();

                            f->setScale(2, 2);
                            
                            window.draw(*f);
                            window.display();
                        } else {
                            r[PPU_STAT] &= 0xfc;
                            r[PPU_STAT] |= 2;
                        }
                    }
                } break;

                // VBlank mode
                case 1: {
                    if (clk >= 456) {
                        clk = 0;
                        r[PPU_LY]++;

                        if (r[PPU_LY] > 153) {
                            r[PPU_STAT] &= 0xfc;
                            r[PPU_STAT] |= 2;
                            r[PPU_LY] = 0;
                        }
                    }
                } break;

                // OAM read period
                case 2: {
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
        }
    }
}