#pragma once

#include "../../aliases.hpp"
#include "../../global.hpp"
#include "../../log.hpp"

#include "memory.hpp"
#include "square.hpp"

namespace gameboy {
    namespace spu {
        u8 counter = 0;
        void update() {
            int16_t ch1_sample = ch1.get_sample(),
                    ch2_sample = ch2.get_sample();

            mixed = ((ch1_sample / 2) + (ch2_sample / 2));

            SDL_QueueAudio(dev, &mixed, sizeof(int16_t));
        }

        void init() {
            desired.freq        = SPU_SAMPLERATE;
            desired.format      = AUDIO_S16SYS;
            desired.channels    = 1;
            desired.samples     = 0x400;
            desired.callback    = nullptr;
            desired.userdata    = NULL;

            dev = SDL_OpenAudioDevice(nullptr, NULL, &desired, nullptr, NULL);

            ch1.init(nr[0x0]);
            ch2.init(nr[0x5]);

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
        }

        u32 read(u16 addr, size_t size) {
            return utility::default_mb_read(nr.data(), addr, size, SPU_BEGIN);
        }
    }
}