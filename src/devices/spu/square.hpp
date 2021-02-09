#pragma once

#include "common.hpp"

#include <cmath>

namespace gameboy {
    namespace spu {
        int16_t generate_square_sample(double t, double f, double a, double dc) {
            if ((!dc) || (!f) || (!a)) return 0x0;

            double c = (SPU_SAMPLERATE * 2) / f,
                   h = c / dc;
                   
            if (!((u32)std::floor(c))) return 0x0;

            double s = detail::sign(((u32)std::floor(t) % (u32)std::floor(c)) - h);
            return s * (a * 0x7fff);
            //t /= SPU_SAMPLERATE;
            //return detail::sign(std::sin((2.0 * M_PI) * f * t)) * (a * 0x7fff);
        }
        
        struct square_t {
            u8* nr = nullptr;

            struct current_sound_t {
                bool playing = false,
                     infinite = false,
                     env_enabled = false;

                size_t remaining_samples = 0,
                       env_step_count = 0,
                       env_step_length = 0,
                       env_step_remaining = 0;

                double env_step = 0.0;
                bool   env_direction = 0;

                double current_freq = 0.0, current_amp = 0.0, dc = 0.0;
            } cs;

            int16_t get_sample() {
                if (cs.playing) {
                    if (cs.infinite ? cs.infinite : (cs.remaining_samples--)) {
                        int16_t sample = generate_square_sample(clk++, cs.current_freq, cs.current_amp, cs.dc);

                        if (cs.env_enabled) {
                            if ((!(cs.env_step_remaining--)) && cs.env_step_count) {
                                if (cs.env_direction) {
                                    cs.current_amp += cs.env_step;
                                } else {
                                    cs.current_amp -= cs.env_step;
                                }
                                if (cs.current_amp < 0) { cs.current_amp = 0.0; cs.playing = false; }
                                cs.env_step_remaining = cs.env_step_length;
                                cs.env_step_count--;
                            }
                        }

                        return sample;

                        if (SDL_GetQueuedAudioSize(dev) > 0x10000) SDL_ClearQueuedAudio(dev);
                    } else {
                        cs.playing = false;
                    }
                } else {
                    return 0;
                }
            }

            void update_state() {
                if (TEST_REG(SPUNR_CTRL, CTRL_RESTR)) {
                    SDL_ClearQueuedAudio(dev);

                    u16 rf = nr[SPUNR_FREQ] | ((nr[SPUNR_CTRL] & 0x7) << 8);

                    bool i = !(nr[SPUNR_CTRL] & CTRL_LENCT);

                    double f = 131072.0 / (2048.0 - (double)rf),
                           a = ((nr[SPUNR_ENVC] & ENVC_STVOL) >> 4) / 16.0;

                    size_t envc = (nr[SPUNR_ENVC] & ENVC_ENVSN),
                           envl = ((double)envc / 64.0) * SPU_SAMPLERATE;
                    bool   envd = nr[SPUNR_ENVC] & ENVC_DIRCT;

                    double envs = 1 / (double)(envc ? envc : 1);

                    size_t l = ((double)(64 - (nr[SPUNR_LENC] & LENC_LENCT)) / 256) * SPU_SAMPLERATE;

                    double dc = duty_cycles[nr[SPUNR_LENC & LENC_WDUTY] >> 6];
                                
                    cs = {
                        true,   // cs.playing
                        i,      // cs.infinite
                        envc,   // cs.env_enabled
                        l,      // cs.remaining_samples
                        envc,   // cs.env_step_count
                        envl,   // cs.env_step_length
                        envl,   // cs.env_step_remaining
                        envs,   // cs.env_step
                        envd,   // cs.env_direction
                        f,      // cs.current_freq
                        a,      // cs.current_amp
                        dc      // cs.dc
                    };
                }
            }

            void init(u8& nr){
                this->nr = &nr;
            }
        } ch1, ch2;
    }
}