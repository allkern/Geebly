#pragma once

#include "cpu.hpp"

namespace gameboy {
    namespace cpu_thread {
        sf::Thread* thread = nullptr;

        static void func() {
            bool exec = true;
            while (exec && !window_closed) {
                cpu::fetch();
                exec = cpu::execute();
            }
        }

        void init() {
            thread = new sf::Thread(&func);
            thread->launch();
        }
    }
}