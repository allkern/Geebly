#pragma once

#include "geebly/gameboy.hpp"

#include "lgw/framebuffer.hpp"

#include <GL/gl3w.h>

#include <array>

#define TILESET_WIDTH 128

namespace frontend {
    namespace debug {
        namespace tileset {
            uint8_t *vram[2] = { nullptr, nullptr },
                    *oam = nullptr;

            lgw::framebuffer <8, 8> tile;

            struct tile_t {
                uint32_t* ptr = nullptr;

                size_t tile = 0;
            };
            
            size_t sx, sy;

            uint8_t* bgp = nullptr;
            uint32_t* dmg_palette = nullptr, *ts_fb = nullptr;

            void render(size_t idx, int bank) {
                uint16_t ri = 0;

                uint8_t ci = (idx & 0xf) << 4;

                ri = (idx & 0x1f0) << 4;

                for (int r = 0; r < 8; r++) {
                    uint16_t rd = *reinterpret_cast<uint16_t*>(vram[bank] + ri + ci + (r << 1));

                    for (int p = 0; p < 8; p++) {
                        tile.draw(p, r, dmg_palette[_pext_u32(rd, 0x8080 >> p)]);
                    }
                }
            }

            static constexpr const size_t width() { return TILESET_WIDTH; }
            static constexpr const size_t height() { return 192; }

            void init() {
                vram[0] = ppu::vram[0].data();
                vram[1] = ppu::vram[1].data();

                oam = ppu::oam.data();
                bgp = &ppu::r[PPU_BGP];
                dmg_palette = ppu::dmg_palette.data();
                ts_fb = tile.get_buffer();
            }
        }
    }
}