#pragma once

#include <cmath>

#include "common.hpp"

namespace gameboy {
    namespace cart {
        struct fm_channel_t {
            double a;

            struct fm_operator_t {
                double a = 0.0, f = 0.0;

                enum adsr_state_t : int {
                    AS_NONE,
                    AS_ATTACK,
                    AS_DECAY,
                    AS_SUSTAIN,
                    AS_RELEASE,
                    AS_DONE
                };

                struct adsr_t {
                    int m_samples = 0;
                    double m_step = 0.0;

                    adsr_state_t state = AS_DONE;

                    double a = 0.0,
                           d = 0.0,
                           s = 0.0,
                           r = 0.0;

                    double peak_level = 0.0,
                           sustain_level = 0.0,
                           base_level = 0.0;

                    bool enabled = true;
                } adsr;

                bool enabled = false;

                double get_sample(double t, bool carrier = false, double fm = 0.0) {
                    if (adsr.enabled) {
                        if (!adsr.m_samples) {
                            switch (adsr.state) {
                                case AS_NONE: {
                                    adsr.state = AS_ATTACK;

                                    a = adsr.base_level;

                                    adsr.m_samples = AEC1_SAMPLERATE * (adsr.a / 1000.0);
                                    adsr.m_step    = (adsr.peak_level - adsr.base_level) / (double)adsr.m_samples;
                                } break;
                                case AS_ATTACK: {
                                    adsr.state = AS_DECAY;

                                    a = adsr.peak_level;

                                    adsr.m_samples = AEC1_SAMPLERATE * (adsr.d / 1000.0);
                                    adsr.m_step    = (adsr.sustain_level - adsr.peak_level) / (double)adsr.m_samples;
                                } break;
                                case AS_DECAY: {
                                    adsr.state = AS_SUSTAIN;

                                    a = adsr.sustain_level;

                                    adsr.m_samples = AEC1_SAMPLERATE * (adsr.s / 1000.0);
                                    adsr.m_step    = 0;
                                } break;
                                case AS_SUSTAIN: {
                                    adsr.state = AS_RELEASE;

                                    adsr.m_samples = AEC1_SAMPLERATE * (adsr.r / 1000.0);
                                    adsr.m_step    = (adsr.base_level - adsr.sustain_level) / (double)adsr.m_samples;

                                    a = adsr.sustain_level;
                                } break;
                                case AS_RELEASE: {
                                    adsr.state = AS_DONE;

                                    a = 0.0;

                                    adsr.m_samples = 0;
                                    adsr.m_step = 0;
                                } break;
                            }
                        }

                        a += adsr.m_step;

                        if (adsr.m_samples) adsr.m_samples--;
                    }

                    double phase = carrier ? fm : 0.0;
                    double amp = carrier ? a : 1.0;

                    return enabled ? std::sin(phase + t * f * AEC1_PI / AEC1_SAMPLERATE) * amp : 0.0;
                }
            };

            fm_operator_t operators[4];

            double get_sample(double t) {
                double v[4];

                v[0] = operators[0].get_sample(t);
                v[1] = operators[1].get_sample(t, true, v[0]);
                v[2] = operators[2].get_sample(t, true, v[1]);
                v[3] = operators[3].get_sample(t, true, v[2]);

                return v[3] * a;
            }
        };
    }
}