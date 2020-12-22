#pragma once

#include "../aliases.hpp"

#define MMIO_DIV 0xff04
#define TIMER_BEGIN 0xff04
#define TIMER_END 0xff07

namespace gameboy {
    namespace timer {
        u8* cpu_last_time = nullptr;

        u8 div = 0x0;

        u32 read(u16 addr, size_t size) {
            if (addr == MMIO_DIV) { return div; }

            return 0;
        }

        void init(u8& cpu_lt) {
            cpu_last_time = &cpu_lt;
        }

        void update() {
            div += (*cpu_last_time) >> 2;
        }
    }
}