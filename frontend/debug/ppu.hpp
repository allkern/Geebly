#pragma once

#include "global.hpp"
#include "tileset.hpp"

#include "geebly/gameboy.hpp"

#include "imgui.h"

namespace frontend {
    namespace debug {
        namespace ppu_panel {
            GLuint tex = 0;
            uint32_t* fb = nullptr;

            void init() {
                fb = ppu::get_buffer();

                glGenTextures(1, &tex);
                glBindTexture(GL_TEXTURE_2D, tex);

                // Setup filtering parameters for display
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

                tileset::init();
            }

            void render() {
                using namespace ImGui;
                //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, PPU_WIDTH, PPU_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, frame_ptr);

                //tileset::update();

                SetNextWindowPos(ImVec2(PPU_PANEL_X, PPU_PANEL_Y));
                SetNextWindowSize(ImVec2(PPU_PANEL_WIDTH, PPU_PANEL_HEIGHT));

                Begin("PPU", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
                End();
            }
        }
    }
}