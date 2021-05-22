#pragma once

#include "common.hpp"

#include <cmath>

namespace gameboy {
    namespace spu {
        int16_t generate_square_sample(double t, double f, double a, double dc) {
            if ((!dc) || (!f) || (!a)) return 0x0;

            double c = SPU_NATIVE_SAMPLERATE / f,
                   h = c / dc;
                   
            if (!((u32)std::round(c))) return 0x0;

            double s = detail::sign(((u32)std::round(t) % (u32)std::round(c)) - h);
            return s * (a * 0x7fff);
        }

        struct square_t {
            double clk = 0.0;

            u8* nr = nullptr;

            struct current_sound_t {
                bool playing = false,
                     infinite = false,
                     env_enabled = false,
                     sweep_enabled = false;

                size_t remaining_samples = 0,
                       env_step_count = 0,
                       env_step_length = 0,
                       env_step_remaining = 0,
                       sweep_step_count = 0,
                       sweep_step_length = 0,
                       sweep_step_n = 0,
                       sweep_step_remaining = 0;

                double env_step = 0.0,
                       sweep_step = 0.0,
                       last_freq = 0.0;

                bool   env_direction = false,
                       sweep_direction = false;

                double current_freq = 0.0, current_amp = 0.0, dc = 0.0;
            } cs;

            int16_t get_sample() {
                if (cs.playing) {
                    if (cs.infinite ? cs.infinite : (cs.remaining_samples--)) {
                        int16_t sample = generate_square_sample(clk++, cs.current_freq, cs.current_amp, cs.dc);

                        // To-do: Correctly implement ch1 sweep
                        // if (cs.sweep_enabled) {
                        //     if (!(cs.sweep_step_remaining--) && (cs.sweep_step_n != cs.sweep_step_count - 1)) {
                        //         if (cs.sweep_direction) {
                        //             cs.sweep_step = cs.last_freq - cs.last_freq / std::pow(2, cs.sweep_step_count + 2);
                        //             cs.current_freq -= cs.sweep_step;
                        //         } else {
                        //             cs.sweep_step = cs.last_freq + cs.last_freq / std::pow(2, cs.sweep_step_count + 2);
                        //             cs.current_freq += cs.sweep_step;
                        //         }

                        //         cs.last_freq = cs.current_freq;
                        //         cs.sweep_step_remaining = cs.sweep_step_length;

                        //         cs.sweep_step_n++;
                        //     }

                        //     if (cs.sweep_step_n == cs.sweep_step_count - 1) cs.playing = false;
                        // }

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
                    } else {
                        cs.playing = false;
                        return 0;
                    }
                } else {
                    return 0;
                }
            }

            void update_state() {
                if (TEST_REG(SPUNR_CTRL, CTRL_RESTR)) {
                    u16 rf = (nr[SPUNR_FREQ] | ((nr[SPUNR_CTRL] & 0x7) << 8));

                    bool i = !(nr[SPUNR_CTRL] & CTRL_LENCT);

                    double f = 131072.0 / (2048.0 - (double)rf),
                           a = ((nr[SPUNR_ENVC] & ENVC_STVOL) >> 4) / 16.0;

                    size_t envc = (nr[SPUNR_ENVC] & ENVC_ENVSN),
                           envl = ((double)envc / 64.0) * (SPU_NATIVE_SAMPLERATE << 2);
                    bool   envd = nr[SPUNR_ENVC] & ENVC_DIRCT;

                    // Prevent SIGFPE
                    double envs = 1 / (double)(envc ? envc : 1);

                    int swpt = (nr[SPUNR_SWPC] >> 4) & 0x7;
                    size_t swpl = ((7.8 * swpt) / 1000.0) * SPU_NATIVE_SAMPLERATE,
                           swpc = (nr[SPUNR_SWPC] & 0x7);
                    bool swpd = (nr[SPUNR_SWPC] >> 3) & 0x1;

                    size_t l = ((64.0 - (double)(nr[SPUNR_LENC] & LENC_LENCT)) / 256.0) * (SPU_NATIVE_SAMPLERATE << 2);

                    double dc = duty_cycles[(nr[SPUNR_LENC] & LENC_WDUTY) >> 6];

                    cs = {
                        true,   // cs.playing
                        i,      // cs.infinite
                        envc,   // cs.env_enabled
                        swpt,   // cs.sweep_enabled
                        l,      // cs.remaining_samples
                        envc,   // cs.env_step_count
                        envl,   // cs.env_step_length
                        envl,   // cs.env_step_remaining
                        swpc,   // cs.sweep_step_count
                        swpl,   // cs.sweep_step_length
                        0,      // cs.sweep_step_n
                        swpl,   // cs.sweep_step_remaining
                        envs,   // cs.env_step
                        0.0,    // cs.sweep_step
                        f,      // cs.last_sweep_step
                        envd,   // cs.env_direction
                        swpd,   // cs.sweep_direction
                        f,      // cs.current_freq
                        a,      // cs.current_amp
                        dc      // cs.dc
                    };
                }
            }

            void init(u8& nr){
                this->nr = &nr;
            }

            void reset() {
                std::memset(&cs, 0, sizeof(current_sound_t));

                for (size_t i = 0; i < 5; i++) nr[i] = 0xff;
            }
        } ch1, ch2;
    }
}