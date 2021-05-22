#pragma once

#include "../global.hpp"
#include "../aliases.hpp"

#define MMIO_KEY1 0xff4d

namespace gameboy {
    namespace clock {
        bool switch_wait = false, double_speed = false;

        size_t scale = 1;
        u8* clock;

        bool do_switch() {
            if (switch_wait && settings::cgb_mode) {
                switch_wait = false;
                double_speed = !double_speed;
                scale *= (double_speed ? 2 : 0.5);
                return true;
            }

            return false;
        }

        // Implement KEY1 here (makes sense)
        u8 read(u16 addr, size_t size) {
            return 0x7e | (u8)double_speed << 7 | (u8)switch_wait;
        }

        void write(u16 addr, u16 value, size_t size) { 
            switch_wait = value & 0x1;
        }

        void init(u8& cpu_clock) {
            clock = &cpu_clock;
        }

        void set_scale(size_t s) {
            scale = s;
        } 

        inline u8 get(size_t scale_override = 0) {
            return (*clock) >> (double_speed ? 2 : 0);
        }
    }
}