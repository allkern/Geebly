#include "../../log.hpp"

#include "memory.hpp"
#include "screen.hpp"

#include "../clock.hpp"
#include "../ic.hpp"

#include <queue>
#include <array>
#include <cstdint>
#include <algorithm>

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

// CGB mode
#define SPATTR_TVBK 0b00001000
#define SPATTR_PALN 0b00000111

namespace gameboy {
    namespace ppu {
        void init() {
            r[PPU_STAT] = 0x82;
        }

        #define TEST_REG(reg, mask) (r[reg] & mask)

        inline void refetch() {
            //sx = cx + r[PPU_SCX];
            sy = r[PPU_LY] + r[PPU_SCY];

            u8 tile_scx_off = (sx >> 3) % 0x20,
               tile_scy_off = (sy >> 3) << 5,
               tile = vram[0][0x1800ul + tile_scx_off + tile_scy_off];

            u16 coff = (tile << 4) + ((sy % 8) << 1);

            l = vram[0][coff],
            h = vram[0][coff + 1];
        }

        void render_row() {
            bool window = TEST_REG(PPU_LCDC, LCDC_WNDSWI) && (r[PPU_LY] >= r[PPU_WY]);

            refetch();
            
            auto get_bit = [](u8 l, size_t s) -> u8 {
                return (l >> s) & 0x1;
            };

            do {
                sx = cx + r[PPU_SCX];
                
                if (!(sx % 8)) refetch();

                size_t s = 7 - (sx % 8);

                u8 color = (get_bit(h, s) << 1) | get_bit(l, s);

                background_fifo.push({
                    color,
                    0,
                    0,
                    false
                });

                rx = (++cx) % 8;
            } while (rx);
        }

        bool vbl_triggered = false;

        #define SWITCH_MODE(m) r[PPU_STAT] = (r[PPU_STAT] & ~(STAT_CRMODE)) | m

        void cycle() {
            //_log(debug, "cx=%lu", cx);

            switch (r[PPU_STAT] & STAT_CRMODE) {
                case MODE_SPR_SEARCH: {
                    // Queue up sprites for this scanline

                    if (clk >= 80) {
                        SWITCH_MODE(MODE_DRAW);
                    }
                } break;

                case MODE_DRAW: {
                    render_row();

                    for (size_t i = 0; i < 8; i++) {
                        // Discard pixels outside the screen (also saves CPU time)
                        
                        frame.draw(
                            fx++, r[PPU_LY],
                            dmg_palette.at(background_fifo.front().color)
                        );

                        background_fifo.pop();
                    }

                    if (clk >= (172 + 80)) {
                        SWITCH_MODE(MODE_HBLANK);
                    }
                } break;

                case MODE_HBLANK: {
                    if (clk >= (172 + 80 + 204)) {
                        r[PPU_LY]++;

                        clk = 0;
                        cx = 0;

                        if (r[PPU_LY] == 144) {
                            SWITCH_MODE(MODE_VBLANK);

                            screen::update();
                        } else {
                            fx = 0;
                            
                            SWITCH_MODE(MODE_SPR_SEARCH);
                        }
                    }
                } break;

                case MODE_VBLANK: {
                    if (!vbl_triggered) {
                        ic::ref(MMIO_IF) |= 0x1;

                        vbl_triggered = true;
                    }

                    if (clk >= (172 + 80 + 204)) {
                        r[PPU_LY]++;

                        clk = 0;

                        if (r[PPU_LY] == 154) {
                            fx = 0;

                            vbl_triggered = false;
                            
                            SWITCH_MODE(MODE_SPR_SEARCH);
                        }
                    }
                } break;
            }

            clk += clock::get();
        }
    }
}

#undef TEST_REG
