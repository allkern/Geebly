#pragma once

#include "common.hpp"

#include <cmath>

namespace gameboy {
    namespace spu {
        uint16_t lfsr = 0x0;

        int get_lfsr_bit(bool lfsr_width) {
            unsigned high_bit_mask = lfsr_width ? 0x4040 : 0x4000;
            bool new_high_bit = (lfsr ^ (lfsr >> 1) ^ 1) & 1;

            lfsr >>= 1;
            
            if (new_high_bit) {
                lfsr |= high_bit_mask;
            } else {
                /* This code is not redundent, it's relevant when switching LFSR widths */
                lfsr &= ~high_bit_mask;
            }
            
            return lfsr & 1;
        }

        int16_t r = 0;   

        int16_t generate_noise_sample(double t, double f, double a, bool w) {
            if ((!f) || (f >= SPU_NATIVE_SAMPLERATE) || (!a)) return 0x0;

            double c = SPU_NATIVE_SAMPLERATE / f;
            
            if (!(((u32)std::round(t) % (u32)std::round(c)))) r = get_lfsr_bit(w);

            double dr = r ? 0.5 : -0.5;

            return 0x3fff - (dr * (a * 0x7fff));
        }
        
        struct noise_t {
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

                double current_freq = 0.0, current_amp = 0.0;

                bool lsfr_width = false;
            } cs;

            void reset() {
                std::memset(&cs, 0, sizeof(current_sound_t));

                r = 0;

                if (nr) for (size_t i = 0; i < 5; i++) nr[i] = 0x00;
            }

            int16_t get_sample() {
                if (cs.playing) {
                    if (cs.infinite ? cs.infinite : (cs.remaining_samples--)) {
                        int16_t sample = generate_noise_sample(clk++, cs.current_freq, cs.current_amp, cs.lsfr_width);

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
                        // Super mega hack:
                        // For some reason, setting CH4's cs.playing flag
                        // to false, actually mutes CH3 (????)
                        // So, we actually set its frequency to 0, so the
                        // sample synth doesn't generate any sound.
                        cs.current_freq = 0.0;
                        //cs.playing = false;

                        return 0x7fff;
                    }
                } else {
                    return 0x7fff;
                }
            }

            void update_state() {
                if (TEST_REG(SPUNR_CTRL, CTRL_RESTR)) {
                    lfsr = 0x0;

                    bool i = !(nr[SPUNR_CTRL] & CTRL_LENCT);

                    double r = nr[SPUNR_FREQ] & 0x7,
                           s = (nr[SPUNR_FREQ] & 0xf0) >> 4;

                    double f = 524288.0 / (!r ? 0.5 : r) / std::pow(2, s + 1),
                           a = ((nr[SPUNR_ENVC] & ENVC_STVOL) >> 4) / 16.0;
                    bool   w = nr[SPUNR_FREQ] & 0x8;
                    
                    size_t envc = 15,
                           envl = ((double)(nr[SPUNR_ENVC] & ENVC_ENVSN) / 64.0) * SPU_NATIVE_SAMPLERATE;
                    bool   envd = nr[SPUNR_ENVC] & ENVC_DIRCT;
                    double envs = (envd ? (1.0 - a) : a) / 16.0;


                    size_t l = ((double)(64 - (nr[SPUNR_LENC] & LENC_LENCT)) / 256) * SPU_NATIVE_SAMPLERATE;

                    if (!nr[SPUNR_ENVC]) reset();

                    cs = {
                        true,   // cs.playing
                        i,      // cs.infinite
                        envl > 0,   // cs.env_enabled
                        l,      // cs.remaining_samples
                        envc,   // cs.env_step_count
                        envl,   // cs.env_step_length
                        envl,   // cs.env_step_remaining
                        envs,   // cs.env_step
                        envd,   // cs.env_direction
                        f,      // cs.current_freq
                        a,      // cs.current_amp
                        w       // cs.lsfr_width
                    };
                }
            }

            void update() {
                lfsr = 0x0;

                if (!(nr[SPUNR_ENVC] & ENVC_STVOL)) {
                    cs.playing = false; return;
                }

                double r = nr[SPUNR_FREQ] & 0x7,
                       s = (nr[SPUNR_FREQ] & 0xf0) >> 4;
                bool   w = nr[SPUNR_FREQ] & 0x8;

                cs.lsfr_width = w;
                cs.current_freq = 524288.0 / (!r ? 0.5 : r) / std::pow(2, s + 1);
            }

            void init(u8& nr){
                this->nr = &nr;
            }

            void save_state(std::ofstream& o) {
                GEEBLY_WRITE_VARIABLE(r);

                o.write(reinterpret_cast<char*>(&cs), sizeof(cs));
            }

            void load_state(std::ifstream& i) {
                GEEBLY_LOAD_VARIABLE(r);

                i.read(reinterpret_cast<char*>(&cs), sizeof(cs));
            }
        } ch4;
    }
}