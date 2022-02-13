#pragma once

#include "common.hpp"

#include <cmath>

namespace gameboy {
    namespace spu {
        int16_t generate_square_sample(double t, double f, double a, double dc, double& cycle) {
            if ((!dc) || (!f) || (!a)) return 0x0;

            double c = SPU_NATIVE_SAMPLERATE / f,
                   h = c / dc;

            cycle = std::fmod(t, h);
                   
            if (!((u32)std::round(c))) return 0x0;

            double s = detail::sign(std::fmod(t, c) - h);

            return s * (a * 0x7fff);
        }

        struct square_t {
            double cycle = 0.0;
            bool pending_freq_change = false;
            double new_freq = 0.0;

            enum square_channel_t : int {
                CH1,
                CH2
            };

            int channel = CH1;

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
                       sweep_step_count = 0, // Count of freq changes
                       sweep_step_length = 0, // Wait this many samples each change
                       sweep_step_n = 0,
                       sweep_step_remaining = 0; // Counter of samples remaining

                double env_step = 0.0,
                       sweep_step = 0.0;
                int    last_freq = 0;

                bool   env_direction = false,
                       sweep_direction = false;

                double current_freq = 0.0, current_amp = 0.0, dc = 0.0;
            } cs;

            void reset() {
                std::memset(&cs, 0, sizeof(current_sound_t));

                if (nr) for (size_t i = 0; i < 5; i++) nr[i] = 0x0;
            }

            int16_t get_sample() {
                if (cs.playing) {
                    // if ((channel == CH1) && cs.env_step_count && cs.env_direction)
                    // _log(debug, "ch1 env update: count=%u, length=%u, direction=%s, current amp=%f, step=%f",
                    //     cs.env_step_count,
                    //     cs.env_step_length, cs.env_direction ? "up" : "down", cs.current_amp, cs.env_step
                    // );
                    if (cs.infinite ? cs.infinite : (cs.remaining_samples--)) {
                        if (pending_freq_change) {
                            if (std::round(cycle) == 0.0) {
                                if (!cs.sweep_enabled || (channel == CH2))
                                    cs.current_freq = new_freq;
                                pending_freq_change = false;
                            }
                        }
                        
                        int16_t sample = generate_square_sample(clk++, cs.current_freq, cs.current_amp, cs.dc, cycle);

                        if (cs.sweep_enabled && cs.sweep_step_count && (channel == CH1)) {
                            if (!cs.sweep_step_remaining) {
                                int lf = cs.last_freq;

                                if (cs.sweep_direction) {
                                    cs.last_freq -= cs.last_freq / std::pow(2, cs.sweep_step_n);
                                } else {
                                    cs.last_freq += cs.last_freq / std::pow(2, cs.sweep_step_n);
                                }

                                cs.current_freq = 131072.0 / (2048.0 - (double)cs.last_freq);

                                cs.sweep_step_remaining = cs.sweep_step_length;

                                cs.sweep_step_count--;
                            } else {
                                cs.sweep_step_remaining--;
                            }
                        } else {
                            if (cs.sweep_enabled && !cs.sweep_step_count) {
                                cs.sweep_enabled = false;
                                //cs.playing = false;
                            }
                        }
                        
                        if (cs.env_enabled) {   
                            if ((!(cs.env_step_remaining--)) && cs.env_step_count) {
                                
                                if (cs.env_direction) {
                                    cs.current_amp += cs.env_step;
                                } else {
                                    cs.current_amp -= cs.env_step;
                                }

                                if (!cs.env_step_count) { cs.current_amp = cs.env_direction ? 1.0 : 0.0; cs.playing = false; }

                                cs.env_step_count--;
                                cs.env_step_remaining = cs.env_step_length;
                            }
                        }

                        return sample;
                    } else {
                        reset();
                        cs.playing = false;
                        return 0;
                    }
                } else {
                    return 0;
                }
            }

            void update() {
                if (!(nr[SPUNR_ENVC] & 0xf8)) {
                    cs.playing = false; return;
                }

                cs.dc = duty_cycles[(nr[SPUNR_LENC] & LENC_WDUTY) >> 6];

                if ((!cs.sweep_enabled) || (channel == CH2)) {
                    u16 rf = (nr[SPUNR_FREQ] | ((nr[SPUNR_CTRL] & 0x7) << 8));
                    cs.current_freq = 131072.0 / (2048.0 - (double)rf);
                    pending_freq_change = true;
                    new_freq = 131072.0 / (2048.0 - (double)rf);
                }
            }

            void update_state() {
                if (TEST_REG(SPUNR_CTRL, CTRL_RESTR)) {
                    u16 rf = (nr[SPUNR_FREQ] | ((nr[SPUNR_CTRL] & 0x7) << 8));

                    bool i = !(nr[SPUNR_CTRL] & CTRL_LENCT);

                    double f = 131072.0 / (2048.0 - (double)rf),
                           a = ((nr[SPUNR_ENVC] & ENVC_STVOL) >> 4) / 16.0;

                    size_t envc = (nr[SPUNR_ENVC] & ENVC_ENVSN) * 2,
                           envl = ((double)envc / 64.0) * SPU_NATIVE_SAMPLERATE;
                    bool   envd = nr[SPUNR_ENVC] & ENVC_DIRCT;

                    // Prevent SIGFPE
                    double envs = a / (double)(envc ? envc : a);

                    if (envd) {
                        envs = (1.0 - a) / (double)(envc ? envc : (1.0 - a));
                    }

                    int    swpt = (nr[SPUNR_SWPC] >> 4) & 0x7;
                    size_t swpl = SPU_NATIVE_SAMPLERATE * ((double)swpt / 128.0),
                           swpc = (nr[SPUNR_SWPC] & 0x7);
                    bool   swpd = (nr[SPUNR_SWPC] >> 3) & 0x1;

                    size_t l = ((64.0 - (double)(nr[SPUNR_LENC] & LENC_LENCT)) / 256.0) * (SPU_NATIVE_SAMPLERATE);

                    double dc = duty_cycles[(nr[SPUNR_LENC] & LENC_WDUTY) >> 6];

                    if (!nr[SPUNR_ENVC]) reset();

                    bool playing = TEST_REG(SPUNR_CTRL, CTRL_RESTR);
                    // if ((channel == CH1) && envc)
                    //     _log(debug, "ch1 env: playing=%u, l=%u, count=%u, length=%u, direction=%s, initial amp=%f, step=%f", playing, l, envc, envl, envd ? "up" : "down", a, envs);

                    cs = {
                        playing,   // cs.playing
                        i,      // cs.infinite
                        envc > 0,   // cs.env_enabled
                        swpt > 0,   // cs.sweep_enabled
                        l,      // cs.remaining_samples
                        envc,   // cs.env_step_count
                        envl,   // cs.env_step_length
                        envl,   // cs.env_step_remaining
                        swpc,   // cs.sweep_step_count
                        swpl,   // cs.sweep_step_length
                        swpc,   // cs.sweep_step_n
                        swpl,   // cs.sweep_step_remaining
                        envs,   // cs.env_step
                        0.0,    // cs.sweep_step
                        rf,     // cs.last_sweep_step
                        envd,   // cs.env_direction
                        swpd,   // cs.sweep_direction
                        f,      // cs.current_freq
                        a,      // cs.current_amp
                        dc      // cs.dc
                    };
                }
            }

            void init(u8& nr, int ch) {
                this->nr = &nr;
                this->channel = ch;
            }

            void save_state(std::ofstream& o) {
                o.write(reinterpret_cast<char*>(&cs), sizeof(cs));
            }

            void load_state(std::ifstream& i) {
                i.read(reinterpret_cast<char*>(&cs), sizeof(cs));
            }
        } ch1, ch2;
    }
}