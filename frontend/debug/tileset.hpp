#pragma once

#include "geebly/gameboy.hpp"

#include "lgw/framebuffer.hpp"

#include <GL/gl3w.h>

#include <thread>

#define TILESET_WIDTH 128

namespace frontend {
    namespace debug {
        namespace tileset {
            uint8_t *vram[2] = { nullptr, nullptr },
                    *oam = nullptr;

            lgw::framebuffer <TILESET_WIDTH, 192> frame;
            
            size_t sx, sy;

            uint8_t* bgp = nullptr;
            uint32_t* dmg_palette = nullptr, *fb = nullptr;

            GLuint tex = 0;

            void render() {
                uint16_t idx = 0, ri = 0;

                while (idx < 0x180) {
                    // Column index and row index
                    uint8_t ci = (idx & 0xf) << 4;

                    ri = (idx & 0x1f0) << 4;

                    for (int r = 0; r < 8; r++) {
                        uint16_t rd = *reinterpret_cast<uint16_t*>(vram[0] + ri + ci + (r << 1));

                        for (int p = 0; p < 8; p++) {
                            frame.draw(p + (ci >> 1), r + (ri >> 5), dmg_palette[_pext_u32(rd, 0x8080 >> p)]);
                        }
                    }

                    idx++;
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
                fb = frame.get_buffer();

                glGenTextures(1, &tex);
                glBindTexture(GL_TEXTURE_2D, tex);

                // Setup filtering parameters for display
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            }

            const GLuint get_image() {
                return tex;
            }

            void update() {
                render();

                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, TILESET_WIDTH, 192, 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, fb);
            }
        }
    }
}