#pragma once

#define LGW_OPTIMIZE
#include "lgw/threaded_window.hpp"

#include "imgui/imgui.h"
#include "imgui/imgui-SFML.h"
#include "imgui/imgui_internal.h"

#include "cpu/registers.hpp"
#include "aliases.hpp"

namespace gameboy {
    class debug_window : public lgw::threaded_window {
        sf::Clock delta;

        cpu::registers::pair* gpr[4] = {
            &cpu::registers::af,
            &cpu::registers::bc,
            &cpu::registers::de,
            &cpu::registers::hl
        };

        u16 *pc = &cpu::registers::pc,
            *sp = &cpu::registers::sp; 
        
        u8  *last_cycles = &cpu::registers::last_instruction_cycles;

        cpu::state* state = &cpu::s;

        void cpu_window() {
            using namespace ImGui;

            
        }

        void setup() override {
            ImGui::CreateContext();
            ImGui::SFML::Init(*get_window());
        }

        void draw() override {
            auto w = get_window();
            ImGui::SFML::Update(*w, delta.restart());
            clear(sf::Color(25, 25, 25));

            ImGui::SFML::Render(*w);
        }

        void on_close() override {
            ImGui::DestroyContext();
            ImGui::SFML::Shutdown();
            close();
        }

    public:
        debug_window() = default;

        inline void start() {
            init(1000, 500, "Geebly Debugger", sf::Style::Default, false, false);
        }
    };
}