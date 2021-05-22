#pragma once

#include "global.hpp"          
#include "geebly/gameboy.hpp"

#include "imgui.h"
#include "imgui_internal.h"

namespace frontend {
    namespace debug {
        namespace cpu_panel {
            void render() {
                using namespace ImGui;

                SetNextWindowPos(ImVec2(CPU_PANEL_X, CPU_PANEL_Y));
                SetNextWindowSize(ImVec2(CPU_PANEL_WIDTH, CPU_PANEL_HEIGHT));

                Begin("CPU", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
                    if (!pause) {
                        PushItemFlag(ImGuiItemFlags_Disabled, true);
                        PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
                    }

                    if (Button("Step"))
                        step = !step;

                    if (!pause) {
                        PopItemFlag();
                        PopStyleVar();
                    }

                    SameLine();

                    if (Button("Pause")) {
                        pause = !pause;

                        master_volume = 1.0 * (!pause);
                    }

                    Separator();

                    using namespace cpu::registers;

                    bool zf_set = r[f] & ZF,
                         nf_set = r[f] & NF,
                         hf_set = r[f] & HF,
                         cf_set = r[f] & CF,
                         vbl_ie     = ic::ie & IRQ_VBL,
                         stat_ie    = ic::ie & IRQ_STAT,
                         timer_ie   = ic::ie & IRQ_TIMER,
                         joypad_ie  = ic::ie & IRQ_JOYP,
                         vbl_if     = ic::ia & IRQ_VBL,
                         stat_if    = ic::ia & IRQ_STAT,
                         timer_if   = ic::ia & IRQ_TIMER,
                         joypad_if  = ic::ia & IRQ_JOYP,
                         vbl_isr    = vbl_ie & vbl_if,
                         stat_isr   = stat_ie & stat_if,
                         timer_isr  = timer_ie & timer_if,
                         joypad_isr = joypad_ie & joypad_if;

                    if (TreeNodeEx("Registers", ImGuiTreeNodeFlags_DefaultOpen)) {

                    Text("af: %04x\ta: %02x\tf: %02x", (u16)af, r[a], r[f]);
                    Text("bc: %04x\tb: %02x\tc: %02x", (u16)bc, r[b], r[c]);
                    Text("de: %04x\td: %02x\te: %02x", (u16)de, r[d], r[e]);
                    Text("hl: %04x\th: %02x\tl: %02x", (u16)hl, r[h], r[l]);
                    Text("pc: %04x", pc);
                    Text("sp: %04x", sp);

                    Separator();

                    Text("Flags:");

                    Checkbox("Z", &zf_set); SameLine();
                    Checkbox("N", &nf_set); SameLine();
                    Checkbox("H", &hf_set); SameLine();
                    Checkbox("C", &cf_set);

                    TreePop();

                    } // cpu#registers TreeNode

                    if (TreeNodeEx("Interrupts", ImGuiTreeNodeFlags_DefaultOpen)) {

                    Text("ime: %s", ime ? "Enabled" : "Disabled");
                    Text("if: %02x", gameboy::ic::ia);
                    Text("ie: %02x", gameboy::ic::ie);

                    Separator();

                    Text("Interrupt table:");
                    Text("   IF   IE    Serviced?");

                    Text("V:"); SameLine(); Checkbox("&", &vbl_if);    SameLine(); Checkbox("->", &vbl_ie);    SameLine(); Checkbox("", &vbl_isr);
                    Text("S:"); SameLine(); Checkbox("&", &stat_if);   SameLine(); Checkbox("->", &stat_ie);   SameLine(); Checkbox("", &stat_isr);
                    Text("T:"); SameLine(); Checkbox("&", &timer_if);  SameLine(); Checkbox("->", &timer_ie);  SameLine(); Checkbox("", &timer_isr);
                    Text("J:"); SameLine(); Checkbox("&", &joypad_if); SameLine(); Checkbox("->", &joypad_ie); SameLine(); Checkbox("", &joypad_isr);

                    TreePop();

                    }

                End();
            }
        }
    }
}