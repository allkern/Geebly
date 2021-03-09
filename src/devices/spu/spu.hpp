#pragma once

#include "../../aliases.hpp"
#include "../../global.hpp"
#include "../../log.hpp"

#include "memory.hpp"
#include "square.hpp"
#include "noise.hpp"
#include "wave.hpp"

namespace gameboy {
    namespace spu {
        inline int16_t mix(int16_t a, int16_t b) {
            return ((a < 0) && (b < 0)) ? ((a + b) - ((a * b) / (std::min(a, b)))) :
                   ((a > 0) && (b > 0)) ? ((a + b) - ((a * b) / (std::max(a, b)))) :
                   (a + b);
        }

        void update() {
            int16_t s1 = ch1.get_sample(),
                    s2 = ch2.get_sample(),
                    s3 = ch3.get_sample(),
                    s4 = ch4.get_sample();

            mixed = (s1 + s2 + s3 + s4) / 4;

            SDL_QueueAudio(dev, &mixed, sizeof(int16_t));
        }

        void init() {
            desired.freq        = SPU_DEVICE_SAMPLERATE;
            desired.format      = AUDIO_S16SYS;
            desired.channels    = 1;
            desired.samples     = 512;
            desired.callback    = nullptr;
            desired.userdata    = nullptr;

            dev = SDL_OpenAudioDevice(nullptr, 0, &desired, nullptr, 0);

            ch1.init(nr[0x0]);
            ch2.init(nr[0x5]);
            ch3.init(nr[0xa]);
            ch4.init(nr[0xf]);

            if (dev) SDL_PauseAudioDevice(dev, 0);
        }

        void write(u16 addr, u16 value, size_t size) {
            // Handle a write to Wave RAM
            if (addr >= 0xff30 && addr <= 0xff3f) {
                utility::default_mb_write(wave_ram.data(), addr, value, size, SPU_BEGIN+0x20);
                return;
            }

            utility::default_mb_write(nr.data(), addr, value, size, SPU_BEGIN);
            
            if (addr == 0xff14) ch1.update_state();
            if (addr == 0xff19) ch2.update_state();
            if (addr == 0xff1e) ch3.update_state();
            if (addr == 0xff23) ch4.update_state();
        }

        u32 read(u16 addr, size_t size) {
            return utility::default_mb_read(nr.data(), addr, size, SPU_BEGIN);
        }
    }
}