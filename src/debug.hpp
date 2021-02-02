#pragma once

//#include "implot.h"
#include "imgui/imgui.h"
#include "imgui/imgui-SFML.h"
#include "imgui/imgui_internal.h"


#include "cpu/registers.hpp"
#include "cpu/mnemonics.hpp"
#include "devices/cart.hpp"
#include "devices/wram.hpp"
#include "devices/hram.hpp"
#include "devices/ppu/memory.hpp"
#include "devices/timer.hpp"
#include "devices/ic.hpp"

#ifdef _WIN32
#include "devices/sound.hpp"
#endif

#include "aliases.hpp"
#include "memory_editor.h"

namespace gameboy {
    namespace debug {
        static bool b = false, d = false, we = false;

        sf::RenderWindow* window = nullptr;
        sf::Clock delta;

        static MemoryEditor mem_edit;
        
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

        static const char* m_memory_combo_items[] = {
            "RVA/BIOS",
            "Cartridge header",
            "ROM0",
            "ROMX",
            "VRA0",
            "VRA1",
            "SRA0",
            "SRAX",
            "WRA0",
            "WRAX",
            "OAM",
            "HRAM"
        };

        size_t item = 0;

        void memory_window() {
            using namespace ImGui;

            SetNextWindowPos(ImVec2(500, 0));
            SetNextWindowSize(ImVec2(515, 400));

            Begin("Memory", nullptr,
                ImGuiWindowFlags_NoDecoration   |
                ImGuiWindowFlags_NoTitleBar     |
                ImGuiWindowFlags_NoCollapse     |
                ImGuiWindowFlags_NoResize       |
                ImGuiWindowFlags_NoMove
            );
                // Memory
                switch (item) {
                    case 0 : mem_edit.DrawContents(cart::rva.data()            , cart::rva.size()      , RVA_BEGIN); break;
                    case 1 : mem_edit.DrawContents(cart::header.data()         , cart::header.size()   , HDR_BEGIN); break;
                    case 2 : mem_edit.DrawContents(cart::cartridge->get_bank0(), 0x4000-0x150          , ROM_BEGIN); break;
                    case 3 : mem_edit.DrawContents(cart::cartridge->get_bank1(), 0x4000                , 0x4000); break;
                    case 4 : mem_edit.DrawContents(ppu::vram[0].data()         , ppu::vram[0].size()   , VRAM_BEGIN); break;
                    case 5 : mem_edit.DrawContents(ppu::vram[1].data()         , ppu::vram[1].size()   , VRAM_BEGIN); break;
                    case 6 : case 7:
                        if (cart::cartridge->get_sram()) {
                            mem_edit.DrawContents(cart::cartridge->get_sram(), 0x2000, 0xa000); 
                        } break;
                    case 8 : mem_edit.DrawContents(wram::wram[0].data()        , wram::wram[0].size()  , WRA0_BEGIN); break;
                    case 9 : mem_edit.DrawContents(wram::current_bank->data()  , wram::current_bank->size(), WRAX_BEGIN); break;
                    case 10: mem_edit.DrawContents(ppu::oam.data()             , ppu::oam.size()       , OAM_BEGIN); break;
                    case 11: mem_edit.DrawContents(hram::hram.data()           , hram::hram.size()     , HRAM_BEGIN); break;
                }

                SameLine();
                if (BeginCombo(
                    m_memory_combo_items[item],
                    m_memory_combo_items[item]
                )) {
                    for (int n = 0; n < 12; n++) {
                        bool is_selected = (item == n);
                        if (Selectable(m_memory_combo_items[n], is_selected))
                            item = n;
                        if (is_selected)
                            SetItemDefaultFocus();
                    }

                    EndCombo();
                }
            End();
        }

        void devices_window() {
            using namespace ImGui;

            SetNextWindowPos(ImVec2(0, 0));
            SetNextWindowSize(ImVec2(501, 400));

            Begin("Devices", nullptr,
                ImGuiWindowFlags_NoDecoration   |
                ImGuiWindowFlags_NoTitleBar     |
                ImGuiWindowFlags_NoCollapse     |
                ImGuiWindowFlags_NoResize       |
                ImGuiWindowFlags_NoMove
            );
                BeginTabBar("main");
                    if (BeginTabItem("CPU")) {
                        if (Button("Step")) {
                            cpu::step = false;
                        }

                        SameLine();

                        Checkbox("Run", &run);

                        cpu::run = run;

                        ImGui::Columns(2);
                        
                        Text("PC: %04x", *pc); NextColumn();
                        Text("SP: %04x", *sp); NextColumn();

                        ImGui::Columns(1);
                        ImGui::Columns(2);
                        
                        Text("Opcode: (%02x) %s", state->opcode, mnemonics[state->opcode].c_str()); NextColumn();

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

                        Text("IF: %02x", ic::read(MMIO_IF, 1)); NextColumn();
                        Text("IE: %02x", ic::read(MMIO_IE, 1)); NextColumn();
                        Text("IME: %s", cpu::registers::ime ? "true" : "false"); NextColumn();

                        EndTabItem();
                    }

                    if (BeginTabItem("PPU")) {
                        Text("LCDC: %02x", ppu::r[PPU_LCDC]);
                        Text("STAT: %02x", ppu::r[PPU_STAT]);
                        Text("LY  : %02x", ppu::r[PPU_LY]);
                        Text("BG Scroll: (%+i (%02x), %+i (%02x))", (int8_t)ppu::r[PPU_SCX], ppu::r[PPU_SCX], (int8_t)ppu::r[PPU_SCY], ppu::r[PPU_SCY]);
                        Text("W Scroll : (%+i (%02x), %+i (%02x))", (int8_t)ppu::r[PPU_WX], ppu::r[PPU_WX], (int8_t)ppu::r[PPU_WY], ppu::r[PPU_WY]);
                        Text("BGP : %02x", ppu::r[PPU_BGP]);
                        EndTabItem();
                    }

                    // WIP Disassembly
                    //for (int i = 0; i < 5; i++)
        // {
        //     float child_height = ImGui::GetTextLineHeight() + style.ScrollbarSize + style.WindowPadding.y * 2.0f;
        //     ImGuiWindowFlags child_flags = ImGuiWindowFlags_HorizontalScrollbar | (enable_extra_decorations ? ImGuiWindowFlags_AlwaysVerticalScrollbar : 0);
        //     ImGuiID child_id = ImGui::GetID((void*)(intptr_t)i);
        //     bool child_is_visible = ImGui::BeginChild(child_id, ImVec2(-100, child_height), true, child_flags);
        //     if (scroll_to_off)
        //         ImGui::SetScrollX(scroll_to_off_px);
        //     if (scroll_to_pos)
        //         ImGui::SetScrollFromPosX(ImGui::GetCursorStartPos().x + scroll_to_pos_px, i * 0.25f);
        //     if (child_is_visible) // Avoid calling SetScrollHereY when running with culled items
        //     {
        //         for (int item = 0; item < 100; item++)
        //         {
        //             if (enable_track && item == track_item)
        //             {
        //                 ImGui::TextColored(ImVec4(1, 1, 0, 1), "Item %d", item);
        //                 ImGui::SetScrollHereX(i * 0.25f); // 0.0f:left, 0.5f:center, 1.0f:right
        //             }
        //             else
        //             {
        //                 ImGui::Text("Item %d", item);
        //             }
        //             ImGui::SameLine();
        //         }
        //     }
        //     float scroll_x = ImGui::GetScrollX();
        //     float scroll_max_x = ImGui::GetScrollMaxX();
        //     ImGui::EndChild();
        //     ImGui::SameLine();
        //     const char* names[] = { "Left", "25%", "Center", "75%", "Right" };
        //     ImGui::Text("%s\n%.0f/%.0f", names[i], scroll_x, scroll_max_x);
        //     ImGui::Spacing();
        // }
                    
                    if (BeginTabItem("Pad")) {
                        Checkbox("Enable", &we); SameLine(); Checkbox("Buttons", &b); SameLine(); Checkbox("D-Pad", &d);
                        
                        if (!(joypad::buttons & JOYP_A)) { Text("A"); SameLine(); }
                        if (!(joypad::buttons & JOYP_B)) { Text("B"); SameLine(); }
                        if (!(joypad::buttons & JOYP_SELECT)) { Text("s"); SameLine(); }
                        if (!(joypad::buttons & JOYP_START)) { Text("S"); SameLine(); }
                        if (!(joypad::buttons & JOYP_UP)) { Text("^"); SameLine(); }
                        if (!(joypad::buttons & JOYP_DOWN)) { Text("v"); SameLine(); }
                        if (!(joypad::buttons & JOYP_LEFT)) { Text("<"); SameLine(); }
                        if (!(joypad::buttons & JOYP_RIGHT)) { Text(">"); SameLine(); }
                        NewLine();
                        Text("Buttons: %02x", joypad::buttons);

                        if (we) joypad::write((!b ? 0x20 : 0x0) | (!d ? 0x10 : 0x0));
                        Text("JOYP: %02x", joypad::read());

                        EndTabItem();
                    }

#ifdef _WIN32
                    if (BeginTabItem("Sound")) {
                        PlotLines("Channel 1", sound::ch1.samples_float.data(), 500, sound::ch1.p, nullptr, 0, 1, ImVec2(0, 80));
                        PlotLines("Channel 2", sound::ch2.samples_float.data(), 500, sound::ch2.p, nullptr, 0, 1, ImVec2(0, 80));
                        PlotLines("Channel 4", sound::ch4.samples_float.data(), 500, sound::ch4.p, nullptr, 0, 1, ImVec2(0, 80));
                        EndTabItem();
                    }
#endif

                    if (BeginTabItem("Timers")) {
                        static const u32 freq[4] = { 0x1000, 0x40000, 0x10000, 0x4000 };
                        if (Button("Step")) {
                            cpu::step = false;
                        }

                        SameLine();

                        Checkbox("Run", &run);

                        cpu::run = run;

                        Text("DIV : %02x (Internal: %04x)", timer::read(MMIO_DIV, 1), timer::div);
                        Text("TIMA: %02x (Internal: %04x)", timer::read(MMIO_TIMA, 1), timer::tima);
                        Text("TMA : %02x", timer::tma);
                        Text("TAC : %02x", timer::read(MMIO_TAC, 1));
                        Text("\tEnable: %i", timer::tac.enable ? 1 : 0);
                        Text("\tFreq: %i Hz (%i kHz)", freq[timer::tac.f], freq[timer::tac.f] >> 0xa);
                        
                        EndTabItem();
                    }

                    if (BeginTabItem("Cartridge")) {
                        Checkbox("Tilt", &tilted_cartridge);
                        EndTabItem();
                    }
                EndTabBar();
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
                    case sf::Event::KeyPressed: { joypad::keydown(event.key.code); } break;
                    case sf::Event::KeyReleased: { joypad::keyup(event.key.code); } break;
                }
            }

            ImGui::SFML::Update(*window, delta.restart());

            devices_window();
            memory_window();

            window->clear(sf::Color(25, 25, 25));
            ImGui::SFML::Render(*window);
            window->display();
        }

        inline void init() {
            window = new sf::RenderWindow(sf::VideoMode(1015, 400), "Geebly Debugger");

            ImGui::CreateContext();

            ImGui::SFML::Init(*window, true);

            ImGuiIO& io = ImGui::GetIO();

            io.Fonts->Clear();
            io.Fonts->AddFontFromFileTTF("res/ubuntu.ttf", 14.0f);
            io.Fonts->AddFontFromFileTTF("res/roboto.ttf", 12.0f);

            ImGui::SFML::UpdateFontTexture();

            ImGui::GetStyle().WindowBorderSize = 0.0f;
            ImGui::GetStyle().WindowRounding = 0.0f;
        }
    };
}