#pragma once

#include "imgui/imgui.h"
#include "imgui/imgui-SFML.h"
#include "imgui/imgui_internal.h"

#include "cpu/registers.hpp"
#include "cpu/mnemonics.hpp"
#include "aliases.hpp"

#include <sys/unistd.h>

namespace gameboy {
    namespace debug {
        sf::RenderWindow* window = nullptr;
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

        bool run = false;

        void cpu_window() {
            using namespace ImGui;

            SetNextWindowSize(ImVec2(400, 300));
            SetNextWindowPos(ImVec2(0, 0));

            Begin("CPU", nullptr,
                ImGuiWindowFlags_NoCollapse |
                ImGuiWindowFlags_NoMove
            );
                if (Button("Step")) {
                    cpu::step = false;
                }

                SameLine();

                if (Checkbox("Run", &run));

                cpu::run = run;

                ImGui::Columns(2);
                
                Text("PC: %04x", *pc); NextColumn();
                Text("SP: %04x", *sp); NextColumn();

                ImGui::Columns(1);
                ImGui::Columns(2);
                
                Text("Opcode: %s (%02x)", mnemonics[state->opcode].c_str(), state->opcode); NextColumn();
                Text("Immediate (16-bit): %04x", state->imm); NextColumn();

                ImGui::Columns(3);

                Text("AF: %04x", gpr[0]->operator gameboy::u16()); NextColumn();
                Text("A: %02x", *gpr[0]->high); NextColumn();
                Text("F: %02x", *gpr[0]->low); NextColumn();
                Text("BC: %04x", gpr[1]->operator gameboy::u16()); NextColumn();
                Text("B: %02x", *gpr[1]->high); NextColumn();
                Text("C: %02x", *gpr[1]->low); NextColumn();
                Text("DE: %04x", gpr[2]->operator gameboy::u16()); NextColumn();
                Text("D: %02x", *gpr[2]->high); NextColumn();
                Text("E: %02x", *gpr[2]->low); NextColumn();
                Text("HL: %04x", gpr[3]->operator gameboy::u16()); NextColumn();
                Text("H: %02x", *gpr[3]->high); NextColumn();
                Text("L: %02x", *gpr[3]->low); NextColumn();
                
            End();
        }

        void update() {
            sf::Event event;
            while (window->pollEvent(event)) {
                ImGui::SFML::ProcessEvent(event);
                switch (event.type) {
                    case sf::Event::Closed: {
                        ImGui::DestroyContext();
                        ImGui::SFML::Shutdown();
                        window->close();
                    } break;
                }
            }

            ImGui::SFML::Update(*window, delta.restart());

            cpu_window();

            window->clear(sf::Color(25, 25, 25));
            ImGui::SFML::Render(*window);
            window->display();
        }

        inline void init() {
            window = new sf::RenderWindow(sf::VideoMode(400, 300), "Geebly Debugger");

            ImGui::CreateContext();
            ImGui::SFML::Init(*window, true);
        }
    };
}