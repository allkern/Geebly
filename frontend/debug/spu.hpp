#pragma once

#include "global.hpp"
#include "tileset.hpp"

#include "geebly/gameboy.hpp"
#include "../audio.hpp"

#include "imgui.h"
#include "implot.h"

namespace frontend {
    namespace debug {
        namespace spu_panel {
            void render() {
                using namespace ImGui;

                ImPlot::ShowDemoWindow();
            }
        }
    }
}