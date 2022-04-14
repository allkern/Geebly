#pragma once

#include "global.hpp"
#include "tileset.hpp"

#include "geebly/gameboy.hpp"

#include "imgui.h"
#include "memory_editor.h"

namespace frontend {
    namespace debug {
        namespace ppu_panel {
            struct tile_t {
                size_t c, r, b;
            } selected = { 0, 0, 0 };

            GLuint ppu_tex[(16 * 24) * 2] = { 0 },
                   st_tex = 0;

            uint32_t* ppu_fb = tileset::tile.get_buffer();

            void bind_texture(GLuint& t) {
                glBindTexture(GL_TEXTURE_2D, t);

                // Setup filtering parameters for display
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            }

            static MemoryEditor cgbpram_edit;

            void init() {
                cgbpram_edit.Cols = 8;
                cgbpram_edit.OptUpperCaseHex = false;
                cgbpram_edit.OptFooterExtraHeight = false;
                cgbpram_edit.OptShowAscii = false;
                cgbpram_edit.OptShowOptions = false;

                glGenTextures((16 * 24) * 2, &ppu_tex[0]);

                for (int i = 0; i < ((16 * 24) * 2); i++) {
                    //bind_texture(ppu_tex[i]);

                    glBindTexture(GL_TEXTURE_2D, ppu_tex[i]);

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
                        tileset::tile.get_buffer()
                    );
                }

                glGenTextures(1, &st_tex);

                glBindTexture(GL_TEXTURE_2D, st_tex);

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
                    tileset::tile.get_buffer()
                );

                tileset::init();
            }

            uint32_t get_palette_color(size_t pal, size_t c, bool sprites) {
                ppu::cgb_palette_ram_t* palette_ram = sprites ? &ppu::cgb_spr_palette : &ppu::cgb_bg_palette;

                uint8_t off = (pal << 3) + (c << 1);

                uint16_t p = palette_ram->at(off) | (palette_ram->at(off+1) << 8);

                uint8_t r = (p & 0x1f) << 3,
                        g = ((p >> 0x5) & 0x1f) << 3,
                        b = ((p >> 0xa) & 0x1f) << 3;

                return lgw::rgb(r, g, b);
            }

            void render_tileset_table(size_t bank) {
                using namespace ImGui;
    
                PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(0.0f, 0.0f));

                char buf[7];

                sprintf(&buf[0], "table%u", bank);

                if (BeginTable(&buf[0], 16, ImGuiTableFlags_Borders | ImGuiTableFlags_NoHostExtendX | ImGuiTableFlags_SizingFixedFit)) 
                {

                for (size_t r = 0; r < 24; r++) {
                    TableNextRow();

                    for (size_t c = 0; c < 16; c++) {
                        TableSetColumnIndex(c);

                        glBindTexture(GL_TEXTURE_2D, ppu_tex[((16 * 24) * bank) + (c + (r * 16))]);
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

                        tileset::render(c + (r * 16), bank);

                        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 8, 8, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, ppu_fb);

                        if (ImageButton((void*)(intptr_t)(ppu_tex[(c + (r * 16)) + ((16 * 24) * bank)]), ImVec2(12, 12), ImVec2(0, 0), ImVec2(1, 1), 1)) {
                            selected = { c, r, bank };
                        }
                    }

                    if ((!((r + 1) % 8)) && (r < 23)) Separator();
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

            bool repeat = false;
            int cgbpram_selected = 0;

            const char* cgbpram_names[] = {
                "Background/Window",
                "Sprites"
            };

            void render() {
                using namespace ImGui;

                SetNextWindowPos(ImVec2(PPU_PANEL_X, PPU_PANEL_Y));
                SetNextWindowSize(ImVec2(PPU_PANEL_WIDTH, PPU_PANEL_HEIGHT));

                Begin("PPU", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
                    BeginChild("selected-tile", ImVec2(170, 360), true);
                        glBindTexture(GL_TEXTURE_2D, st_tex);
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

                        tileset::render(selected.c + (selected.r * 16), selected.b);

                        Text("Selected tile");

                        Separator();

                        Checkbox("Repeat", &repeat);

                        auto before = GetCursorPosX();

                        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 8, 8, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, ppu_fb);

                        int size = 64 * ((int)repeat + 1);

                        SetCursorPosX(((180 - size) * 0.5f) - 5);
                        Image((void*)(intptr_t)st_tex, ImVec2(size, size), ImVec2(0, 0), ImVec2((int)repeat + 1, (int)repeat + 1));

                        SetCursorPosX(before);

                        Separator();

                        uint8_t index = (selected.c + (selected.r << 4)) & 0xff;

                        Text("Index:");
                        Text(" Unsigned: %u (%02x)", index, index);
                        Text(" Signed: %i", (int16_t)(int8_t)index);
                        Text("Address: %04x:%u", 0x8000 + ((selected.c << 4) + (selected.r << 8)), selected.b);
                    EndChild(); SameLine();

                    BeginChild("tileset", ImVec2(0, 360), true);
                        render_tileset_table(0); SameLine();
                        render_tileset_table(1);
                    EndChild();

                    Separator();

                    BeginChild("registers", ImVec2(300, 240));
                        Text("Registers");
                        Separator();
                        Text("lcdc       %02x", ppu::r[PPU_LCDC]);
                        Text("stat       %02x", ppu::r[PPU_STAT]);
                        Text("ly, lyc    %02x -> %02x", ppu::r[PPU_LY], ppu::r[PPU_LYC]);
                        Text("background (%02x (%+i), %02x (%+i))", ppu::r[PPU_SCX], ppu::r[PPU_SCX], ppu::r[PPU_SCY], ppu::r[PPU_SCY]);
                        Text("window     (%02x (%+i), %02x (%+i))", ppu::r[PPU_WX], ppu::r[PPU_WX], ppu::r[PPU_WY], ppu::r[PPU_WY]);
                        Text("clk        %u (mode %u)", ppu::clk, ppu::r[PPU_STAT] & 0x3);
                        Separator();

                        Text("bgp       : %02x", ppu::r[PPU_BGP]);

                        DISPLAY_PALETTE(BGP);

                        Separator();

                        Text("obp0, obp1: %02x, %02x", ppu::r[PPU_OBP0], ppu::r[PPU_OBP1]);

                        DISPLAY_PALETTE(OBP0);
                        DISPLAY_PALETTE(OBP1);
                    EndChild();

                    SameLine();

                    BeginChild("cgb-palettes#master", ImVec2(0, 250));
                        if (BeginCombo("Palette RAM", cgbpram_names[cgbpram_selected])) {
                            for (int n = 0; n < 2; n++) {
                                bool is_selected = (cgbpram_names[cgbpram_selected] == cgbpram_names[n]);
                                
                                if (Selectable(cgbpram_names[n], is_selected))
                                    cgbpram_selected = n;
                                if (is_selected)
                                    SetItemDefaultFocus();
                            }

                            EndCombo();
                        }

                        Separator();

                        BeginChild("cgb-palettes", ImVec2(210, 200));
                            Text("Memory");

                            Separator();

                            if (cgbpram_selected) {
                                cgbpram_edit.DrawContents(ppu::cgb_spr_palette.data(), ppu::cgb_spr_palette.size(), 0);
                            } else {
                                cgbpram_edit.DrawContents(ppu::cgb_bg_palette.data(), ppu::cgb_bg_palette.size(), 0);
                            }
                        EndChild();
                        
                        SameLine();

                        BeginChild("cgb-colors", ImVec2(0, 225));
                            Text("Colors");

                            Separator();

                            for (int p = 0; p < 8; p++) {
                                for (int c = 0; c < 4; c++) {
                                    uint32_t color = get_palette_color(p, c, cgbpram_selected);

                                    ColorButton("CGB Palette Color",
                                        ImVec4(
                                            (float)((color >> 24) & 0xff) / 255.0,
                                            (float)((color >> 16) & 0xff) / 255.0,
                                            (float)((color >> 8 ) & 0xff) / 255.0,
                                            (float)(color         & 0xff) / 255.0
                                        ),
                                        ImGuiColorEditFlags_NoBorder,
                                        ImVec2(12, 12.6)
                                    ); SameLine(0.0, 2.0);
                                }
                                NewLine();
                            }
                        EndChild();
                    EndChild();

                    if (TreeNode("FIFOs")) {
                        TreePop();
                    }
                End();
            }
        }
    }
}