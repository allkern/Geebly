#pragma once

#include "global.hpp"          
#include "geebly/gameboy.hpp"
#include <GL/gl3w.h>
#include "imgui.h"

namespace frontend {
    namespace debug {
        namespace screen_panel {
            int scale = 2;
            GLuint screen_tex = 0;
            uint32_t* screen_fb = nullptr;

            void init() {
                screen_fb = ppu::get_buffer();

                glGenTextures(1, &screen_tex);
                glBindTexture(GL_TEXTURE_2D, screen_tex);

                // Setup filtering parameters for display
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            }

            void render() {
                using namespace ImGui;

                glGenTextures(1, &screen_tex);
                glBindTexture(GL_TEXTURE_2D, screen_tex);

                // Setup filtering parameters for display
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, PPU_WIDTH, PPU_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, screen_fb);

                SetNextWindowPos(ImVec2(WINDOW_WIDTH - (PPU_WIDTH * scale), MENUBAR_SIZE));
                SetNextWindowSize(ImVec2(PPU_WIDTH * scale, TITLEBAR_SIZE + PPU_HEIGHT * scale));
                PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0,0));

                Begin("Screen", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
                    Image((void*)(intptr_t)screen_tex, ImVec2(PPU_WIDTH * scale, PPU_HEIGHT * scale));
                End();

                PopStyleVar();
            }
        }
    }
}