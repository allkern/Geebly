#pragma once

#include "cpu.hpp"

namespace gameboy {
    namespace cpu_thread {
        sf::Thread* thread = nullptr;

        static void func() {
            bool exec = true;
            while (exec && !window_closed) {
                exec = cpu::cycle();
                //if (cpu::registers::pc == 0xff80) { cpu::run = false; }
            }
        }

        void init() {
            thread = new sf::Thread(&func);
            thread->launch();
        }
    }
}