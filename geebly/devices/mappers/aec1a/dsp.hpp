#pragma once

#include "common.hpp"

namespace gameboy {
    namespace cart {
        struct fm_channel_t {
            AEC1A_DSP_TYPE main_amp;
            AEC1A_DSP_TYPE t = 0.0;

            struct lfo_t {
                AEC1A_DSP_TYPE f, a;

                AEC1A_DSP_TYPE get_sample(AEC1A_DSP_TYPE t) {
                    return AEC1A_SIN(t * f * AEC1A_PI / AEC1A_SAMPLERATE) * a;
                }
            } lfo;

            bool lfo_enable;

            enum algorithm_t : int {
                FM_ALG1 = 0,
                FM_ALG2 = 1,
                FM_ALG3 = 2
            };

            algorithm_t algorithm = FM_ALG1;

            struct fm_operator_t {
                AEC1A_DSP_TYPE a = 0.0, f = 0.0;
                AEC1A_DSP_TYPE multiplier = 1.0;
                AEC1A_DSP_TYPE detune = 0.0;

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
                    AEC1A_DSP_TYPE m_step = 0.0;

                    adsr_state_t state = AS_DONE;

                    AEC1A_DSP_TYPE a = 0.0,
                           d = 0.0,
                           s = 0.0,
                           r = 0.0;

                    AEC1A_DSP_TYPE peak_level = 0.0,
                           sustain_level = 0.0,
                           base_level = 0.0;

                    bool enabled = true;
                } adsr;

                bool enabled = false;

                AEC1A_DSP_TYPE get_sample(AEC1A_DSP_TYPE t, bool carrier = false, AEC1A_DSP_TYPE fm = 0.0, bool lfo_enable = false, AEC1A_DSP_TYPE lfo = 0.0) {
                    if (!enabled) return 0.0;

                    if (adsr.enabled) {
                        if (!adsr.m_samples) {
                            switch (adsr.state) {
                                case AS_NONE: {
                                    adsr.state = AS_ATTACK;

                                    a = adsr.base_level;

                                    adsr.m_samples = AEC1A_SAMPLERATE * (adsr.a / 1000.0);
                                    adsr.m_step    = (adsr.peak_level - adsr.base_level) / (AEC1A_DSP_TYPE)adsr.m_samples;
                                } break;
                                case AS_ATTACK: {
                                    adsr.state = AS_DECAY;

                                    a = adsr.peak_level;

                                    adsr.m_samples = AEC1A_SAMPLERATE * (adsr.d / 1000.0);
                                    adsr.m_step    = (adsr.sustain_level - adsr.peak_level) / (AEC1A_DSP_TYPE)adsr.m_samples;
                                } break;
                                case AS_DECAY: {
                                    adsr.state = AS_SUSTAIN;

                                    a = adsr.sustain_level;

                                    adsr.m_samples = AEC1A_SAMPLERATE * (adsr.s / 1000.0);
                                    adsr.m_step    = 0;
                                } break;
                                case AS_SUSTAIN: {
                                    adsr.state = AS_RELEASE;

                                    adsr.m_samples = AEC1A_SAMPLERATE * (adsr.r / 1000.0);
                                    adsr.m_step    = (adsr.base_level - adsr.sustain_level) / (AEC1A_DSP_TYPE)adsr.m_samples;

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

                    AEC1A_DSP_TYPE phase = carrier ? fm : 0.0;
                    AEC1A_DSP_TYPE amp = carrier ? a : 1.0;
                    AEC1A_DSP_TYPE freq = (f * multiplier) + detune + (lfo_enable ? lfo : 0.0);

                    return enabled ? AEC1A_SIN(phase + t * freq * AEC1A_PI / AEC1A_SAMPLERATE) * amp : 0.0;
                }
            };

            fm_operator_t operators[4];

            AEC1A_DSP_TYPE get_sample() {
                AEC1A_DSP_TYPE v[4];

                AEC1A_DSP_TYPE lfo_sample = lfo.get_sample(t);

                switch (algorithm) {
                    case FM_ALG1: {
                        // M1 -> M2 -> M3 -> M4 -> Out
                        v[0] = operators[0].get_sample(t, false, 0.0 , lfo_enable, lfo_sample);
                        v[1] = operators[1].get_sample(t, true , v[0], lfo_enable, lfo_sample);
                        v[2] = operators[2].get_sample(t, true , v[1], lfo_enable, lfo_sample);
                        v[3] = operators[3].get_sample(t, true , v[2], lfo_enable, lfo_sample);
                    } break;

                    case FM_ALG2: {
                        // M1
                        //  +-> M3 -> M4 -> Out
                        // M2 
                        v[0] = operators[0].get_sample(t, false, 0.0        , lfo_enable, lfo_sample);
                        v[1] = operators[1].get_sample(t, false, 0.0        , lfo_enable, lfo_sample);
                        v[2] = operators[2].get_sample(t, true , v[0] + v[1], lfo_enable, lfo_sample);
                        v[3] = operators[3].get_sample(t, true , v[2]       , lfo_enable, lfo_sample);
                    } break;

                    case FM_ALG3: {
                        // M2 -> M3
                        //        +-> M4 -> Out
                        //       M1
                        v[0] = operators[1].get_sample(t, false, 0.0        , lfo_enable, lfo_sample);
                        v[1] = operators[2].get_sample(t, true , v[0]       , lfo_enable, lfo_sample);
                        v[2] = operators[0].get_sample(t, false, 0.0        , lfo_enable, lfo_sample);
                        v[3] = operators[3].get_sample(t, true , v[1] + v[2], lfo_enable, lfo_sample);
                    } break;
                }

                t++;

                return v[3] * main_amp;
            }
        };
    }
}