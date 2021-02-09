#pragma once

#include "common.hpp"
#include "memory.hpp"

#include <cmath>

namespace gameboy {
    namespace spu {
        struct wave_t {
            struct current_sound_t {
                bool playing = false,
                     infinite = false;

                size_t remaining_samples = 0;
                u8 current_sample = 0;

                double amp = 0.0;
            } cs;

            void init(u8& nr) {
                this->nr = &nr;
            }
        };
    }
}