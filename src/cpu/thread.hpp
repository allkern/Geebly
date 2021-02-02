#pragma once

#include "cpu.hpp"

namespace gameboy {
    namespace cpu_thread {
        sf::Thread* thread = nullptr;

        static void func() {
            bool exec = true;
            while (exec && !window_closed) {
                //if (cpu::registers::pc == 0x0042) { cpu::run = false; cpu::step = false; }
                exec = cpu::cycle();
            }
        }

        void init() {
            thread = new sf::Thread(&func);
            thread->launch();
        }
    }
}