#pragma once

#include "../global.hpp"
#include "../aliases.hpp"

#define MMIO_KEY1 0xff4d

namespace gameboy {
    namespace clock {
        bool switch_wait = false, double_speed = false;

        size_t scale = 1;
        int* clock;

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
            if (settings::cgb_mode) {
                return 0x7e | (u8)double_speed << 7 | (u8)switch_wait;
            } else {
                return 0xff;
            }
        }

        void write(u16 addr, u16 value, size_t size) { 
            switch_wait = value & 0x1;
        }

        void reset() {
            switch_wait = false;
            double_speed = false;
            scale = 1;
        }

        void save_state(std::ofstream& o) {
            GEEBLY_WRITE_VARIABLE(switch_wait);
            GEEBLY_WRITE_VARIABLE(double_speed);
            GEEBLY_WRITE_VARIABLE(scale);
        }

        void load_state(std::ifstream& i) {
            GEEBLY_LOAD_VARIABLE(switch_wait);
            GEEBLY_LOAD_VARIABLE(double_speed);
            GEEBLY_LOAD_VARIABLE(scale);
        }

        void init(int& cpu_clock) {
            clock = &cpu_clock;
        }

        void set_scale(size_t s) {
            scale = s;
        } 

        inline u8 get(size_t scale_override = 0) {
            return (*clock) >> (double_speed ? 1 : 0);
        }
    }
}