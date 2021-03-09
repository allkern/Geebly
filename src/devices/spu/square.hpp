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
                    SDL_ClearQueuedAudio(dev);

                    u16 rf = (nr[SPUNR_FREQ] | ((nr[SPUNR_CTRL] & 0x7) << 8));

                    bool i = !(nr[SPUNR_CTRL] & CTRL_LENCT);

                    double f = 131072.0 / (2048.0 - (double)rf),
                           a = ((nr[SPUNR_ENVC] & ENVC_STVOL) >> 4) / 16.0;

                    size_t envc = (nr[SPUNR_ENVC] & ENVC_ENVSN),
                           envl = ((double)envc / 64.0) * SPU_DEVICE_SAMPLERATE * 4;
                    bool   envd = nr[SPUNR_ENVC] & ENVC_DIRCT;

                    // Prevent SIGFPE
                    double envs = 1 / (double)(envc ? envc : 1);

                    size_t l = ((64.0 - (double)(nr[SPUNR_LENC] & LENC_LENCT)) / 256.0) * SPU_DEVICE_SAMPLERATE * 4;

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

                    //_log(debug,
                    //    "full ch1/ch2 channel state:\n\tplaying=%s\n\ti=%s\n\tenvc=%s\n\tlen=%llu\n\tenvc=%llu\n\tenvl=%llu\n\tenvr=%llu\n\tenvs=%f\n\tenvd=%s\n\tf=%f\n\ta=%f\n\tdc=%f",
                    //    cs.playing ? "true" : "false",
                    //    cs.infinite ? "true" : "false",
                    //    cs.env_enabled ? "true" : "false",
                    //    cs.remaining_samples,
                    //    cs.env_step_count,
                    //    cs.env_step_length,
                    //    cs.env_step_remaining,
                    //    cs.env_step,
                    //    cs.env_direction ? "up" : "down",
                    //    cs.current_freq,
                    //    cs.current_amp,
                    //    cs.dc
                    //);
                }
            }

            void init(u8& nr){
                this->nr = &nr;
            }
        } ch1, ch2;
    }
}