#pragma once

#include "cpu/cpu.hpp"

#include <sys/unistd.h>

namespace gameboy {
    sf::Thread* cpu_thread = nullptr;

    void cpu_thread_func() {
        bool exec = true;
        while (exec) {
            cpu::fetch();
            exec = cpu::execute();
            //usleep(1);
            //if (cpu::registers::pc == 0x392/*28030x27ac*/) {
            //    cpu::run = false;
            //    std::cout << std::hex << (unsigned int)bus::read(0x9280, 1) << std::endl;
            //}
        }
    }

    void init() {
        cpu_thread = new sf::Thread(&cpu_thread_func);
        cpu_thread->launch();
    }
}