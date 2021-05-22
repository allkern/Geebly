#pragma once

#include "../../aliases.hpp"

#include <array>

#include "common.hpp"

namespace gameboy {
    namespace spu {
        typedef std::array <u8, 0x17> nr_t;
        typedef std::array <u8, 0x20> wave_ram_t;
        typedef std::array <double, 4> duty_cycles_t;

        static const duty_cycles_t duty_cycles = { 8.0, 4.0, 2.0, 1.3 };

        double so1_output_level, so2_output_level;

        nr_t nr = {
            0x80, 0xbf, 0xf3, 0xff, 0x3f, 0xff,
            0x3f, 0x00, 0xff, 0xbf,
            0x7f, 0xff, 0x9f, 0xff, 0xbf, 0xff,
            0xff, 0x00, 0x00, 0xbf, 0x77,
            0xf3, 0xf1
        };

        static nr_t or_mask = { 0 };

        bool spu_on = true;

        wave_ram_t wave_ram = { 0 };

        int16_t mixed = 0;
    }
}