#pragma once

#include "common.hpp"
#include "../clock.hpp"

#include <cmath>

namespace gameboy {
    namespace spu {
        u8 s = 0, fetched = 0;

        int16_t generate_wave_sample(u16& sample, double t, double f, double a, u8 level) {
            if ((!f) || (!a)) return 0x0;

            double c = ((double)SPU_NATIVE_SAMPLERATE / f),
                   sl = c / 32;

            if (!(uint32_t)sl) sl = 1;

            if (!((uint32_t)std::round(t) % (uint32_t)sl)) {
                s = (wave_ram.at(sample >> 1) >> ((1 - (sample & 0x1)) << 2)) & 0xf;
                sample++;
                sample &= 0x1f;
            }

            int16_t output = (level ? (s >> (level - 1)) : 0) * 0x888;

            return (output > 0x7fff) ? 0x7fff : output;
        }
        
        struct wave_t {
            double clk = 0.0;

            u8* nr = nullptr;

            struct current_sound_t {
                bool playing = false,
                     infinite = false;

                size_t remaining_samples = 0;
                u16    current_sample_idx = 0;
                u8     output_level = 0x0;

                double freq = 0.0, amp = 0.0;
            } cs;

            int16_t get_sample() {
                if (cs.playing) {
                    if (cs.infinite ? true : (cs.remaining_samples--)) {
                        int16_t sample = generate_wave_sample(cs.current_sample_idx, clk++, cs.freq, cs.amp, cs.output_level);

                        return sample;
                    } else {
                        cs.playing = false;
                        return 0;
                    }
                } else {
                    return 0;
                }
            }

            void update_state() {
                if (TEST_REG(SPUNR_CTRL, CTRL_RESTR) || cs.playing) {
                    bool i = !(nr[SPUNR_CTRL] & CTRL_LENCT);

                    u16 rf = (nr[SPUNR_FREQ] | ((nr[SPUNR_CTRL] & CTRL_FREQH) << 8)) & 0x7ff;

                    double f = 65536.0 / (2048.0 - rf);

                    size_t l = ((double)(256 - nr[SPUNR_LENC]) / 256) * (SPU_NATIVE_SAMPLERATE >> 2);

                    u8 o = (nr[0x2] >> 5) & 3;

                    cs = {
                        true,   // cs.playing
                        i,      // cs.infinite
                        l,      // cs.remaining_samples
                        0,      // cs.current_sample_idx
                        o,      // cs.output_level
                        f,      // cs.freq
                        1       // cs.amp
                    };
                }
            }
            
            void update_output_level() {
                cs.output_level = (nr[0x2] >> 5) & 3;
            }

            void init(u8& nr){
                this->nr = &nr;
            }

            void reset() {
                std::memset(&cs, 0, sizeof(current_sound_t));

                s = 0;

                for (size_t i = 0; i < 5; i++) nr[i] = 0xff;
            }
        } ch3;
    }
}