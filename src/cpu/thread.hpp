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

            //if (cpu::registers::pc == 0x100 ) { cpu::run = false; cpu::step = true; }
            //if (cpu::registers::pc == 0x3e85) { cpu::run = false; cpu::step = true; }
            //if (cpu::registers::pc == 0x2058) { cpu::run = false; cpu::step = true; }
            //if (cpu::registers::pc == 0x60a2) { cpu::run = false; cpu::step = true; }
            //if (cpu::registers::pc == 0x0181) { cpu::run = false; cpu::step = true; }
            //if (cpu::registers::pc == 0x00d4) { cpu::run = false; cpu::step = true; }
            //if (cpu::registers::pc == 0x00d7) { cpu::run = false; cpu::step = true; }
              if (cpu::registers::pc == 0x0338) { cpu::run = false; cpu::step = true; }
              if (cpu::registers::pc == 0x0423) { cpu::run = false; cpu::step = true; }
            //if (cpu::registers::pc == 0x00e3) { cpu::run = false; cpu::step = true; }
            //if (cpu::registers::pc == 0x405a) { cpu::run = false; cpu::step = true; }
            //if (cpu::registers::pc == 0x35dc) { cpu::run = false; cpu::step = true; }
            //if (cpu::registers::pc == 0x240f) { cpu::run = false; cpu::step = true; }
            //if (cpu::registers::pc == 0x3e7a) { cpu::run = false; cpu::step = true; }
        }
        
    }

    void init() {
        cpu_thread = new sf::Thread(&cpu_thread_func);
        cpu_thread->launch();
    }
}