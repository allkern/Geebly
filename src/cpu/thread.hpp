#pragma once

#include "cpu.hpp"

#include <sys/unistd.h>

namespace gameboy {
    sf::Thread* cpu_thread = nullptr;

    void cpu_thread_func() {
        bool exec = true;
        while (exec && !window_closed) {
            cpu::fetch();
            exec = cpu::execute();
        }
    }

    void init() {
        cpu_thread = new sf::Thread(&cpu_thread_func);
        cpu_thread->launch();
    }
}