#pragma once

#include "global.hpp"
#include "tileset.hpp"

#include "geebly/gameboy.hpp"

#include "imgui.h"

namespace frontend {
    namespace debug {
        namespace ppu_panel {
            GLuint ppu_tex = 0;
            uint32_t* ppu_fb = tileset::tile.get_buffer();

            void init() {
                glGenTextures(1, &ppu_tex);
                glBindTexture(GL_TEXTURE_2D, ppu_tex);

                // Setup filtering parameters for display
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

                tileset::init();
            }

            void render_tileset_table(size_t bank) {
                using namespace ImGui;
    
                PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(0.0f, 0.0f));

                if (BeginTable("table1", 16, ImGuiTableFlags_Borders | ImGuiTableFlags_NoHostExtendX | ImGuiTableFlags_SizingFixedFit)) 
                {

                for (size_t r = 0; r < 24; r++) {
                    TableNextRow();

                    for (size_t c = 0; c < 16; c++) {
                        TableSetColumnIndex(c);

                        tileset::render(c + (r * 16), bank);
                        
                        glGenTextures(1, &ppu_tex);
                        glBindTexture(GL_TEXTURE_2D, ppu_tex);

                        // Setup filtering parameters for display
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

                        glTexImage2D(
                            GL_TEXTURE_2D,
                            0,
                            GL_RGBA,
                            8, 8,
                            0,
                            GL_RGBA,
                            GL_UNSIGNED_INT_8_8_8_8,
                            ppu_fb
                        );

                        ImageButton((void*)(intptr_t)ppu_tex, ImVec2(12, 12), ImVec2(0, 0), ImVec2(1, 1), 1);
                    }
                }

                EndTable();

                }

                PopStyleVar();
            }

            #define DISPLAY_PALETTE(N) \
                for (int i = 0; i < 4; i++) { \
                    uint32_t color = ppu::dmg_palette[(ppu::r[PPU_##N] >> (i << 1)) & 0x3]; \
                    char title[256]; \
                    sprintf(&title[0], #N " color %u", i); \
                    ColorButton(title, \
                        ImVec4( \
                            (float)((color >> 8 ) & 0xff) / 255.0, \
                            (float)((color >> 16) & 0xff) / 255.0, \
                            (float)((color >> 24) & 0xff) / 255.0, \
                            (float)(color         & 0xff) / 255.0 \
                        ), \
                        0, \
                        ImVec2(20, 20) \
                    ); \
                    SameLine(); \
                } NewLine();

            void render() {
                using namespace ImGui;

                SetNextWindowPos(ImVec2(PPU_PANEL_X, PPU_PANEL_Y));
                SetNextWindowSize(ImVec2(PPU_PANEL_WIDTH, PPU_PANEL_HEIGHT));

                Begin("PPU", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
                    render_tileset_table(0); SameLine();
                    render_tileset_table(1);

                    Separator();

                    Text("lcdc      : %02x", ppu::r[PPU_LCDC]);
                    Text("stat      : %02x", ppu::r[PPU_STAT]);
                    Text("ly, lyc   : %02x -> %02x", ppu::r[PPU_LY], ppu::r[PPU_LYC]);
                    Text("background: (%02x (%+i), %02x (%+i))", ppu::r[PPU_SCX], ppu::r[PPU_SCX], ppu::r[PPU_SCY], ppu::r[PPU_SCY]);
                    Text("window    : (%02x (%+i), %02x (%+i))", ppu::r[PPU_WX], ppu::r[PPU_WX], ppu::r[PPU_WY], ppu::r[PPU_WY]);
                    
                    Separator();

                    Text("bgp       : %02x", ppu::r[PPU_BGP]);

                    DISPLAY_PALETTE(BGP);

                    Separator();

                    Text("obp0, obp1: %02x, %02x", ppu::r[PPU_OBP0], ppu::r[PPU_OBP1]);

                    DISPLAY_PALETTE(OBP0);
                    DISPLAY_PALETTE(OBP1);

                End();
            }
        }
    }
}