#pragma once

#ifdef _WIN32
	#include "SFML\Graphics.hpp"
    #include "SFML\Audio.hpp"
#endif

#ifdef __linux__
    #include <SFML/Audio.hpp>
    #include <SFML/Graphics.hpp>
#endif

#include "../aliases.hpp"
#include "../log.hpp"

#include <iostream>
#include <ctime>
#include <cmath>
#include <array>

#define SAMPLE_RATE 44100
#define SOUND_LEN 0x2000
#define SOUND_BEGIN 0xff10
#define SOUND_END 0xff3f

namespace gameboy {
    namespace sound {
        u8 dummy;

        struct channel {
            std::array <u8, 5> nr = { 0 };
            size_t p;
            sf::SoundBuffer buf;
            sf::Sound out;
            std::array <int16_t, SAMPLE_RATE> samples;
            std::array <float, SAMPLE_RATE> samples_float;

            void push_sample(int16_t sample) {
                if (p < samples.size()) {
                    samples[p] = sample;
                    samples_float[p++] = (float)sample;
                } else { samples_float.fill(0.0f); samples.fill(0); p = 0; }
            }
        };

        channel ch1, ch2, ch3, ch4, mixer;

        //struct synthesizer : public sf::SoundStream {
        //    virtual bool onGetData(Chunk& data) override {
        //        data.samples = 
        //    }
        //
        //    void init() {
        //        initialize(1, SAMPLE_RATE);
        //    }
        //};

        int16_t square_generator(double t, double f, double a) {
            if (f > SAMPLE_RATE) f = SAMPLE_RATE / 2;

            int tpc = SAMPLE_RATE / f,
                cp = int(t) % tpc,
                hf = tpc / 2;

            if (cp < hf) return 0x7fff * a;

            return 0;
        }

        int16_t noise_generator(double t, double f, double a) {
            if (f > SAMPLE_RATE || !(f && a)) return 0;

            int tpc = SAMPLE_RATE / f,
                cp = int(t) % tpc,
                hf = tpc / 2;
            
            bool written = false, value = false;
            
            if (cp < hf) {
                if (!written) {
                    value = rand() > (RAND_MAX / 2);
                    
                    written = true;
                }
                
                return (value * 0x7fff) * a;
            }
            
            if (written) {
                value = rand() > (RAND_MAX / 2);

                written = false;
            }

            return (value * 0x7fff) * a;
        }

        enum class channel_type {
            square,
            wave,
            noise
        };

        std::array <u8, 0x30> nr = { 0 };

        u32 read(u16 addr, size_t size) {
            return 0;
        }

        void trigger(channel* ch, channel_type ct = channel_type::square) {
            if (ch->nr[4] & 0x80) {
                ch->out.stop();
                ch->p = 0;

                // Noise + Square variables
                double l = 0, f = 0, s = 0, r = 0;

                if (ct == channel_type::square) {
                    l = (64u - (ch->nr[1] & 0x3f));
                    f = 0x20000 / (0x800 - (((ch->nr[4] & 0x7) << 8) | ch->nr[3]));
                }

                if (ct == channel_type::noise) {
                    l = (64u - (ch->nr[1] & 0x3f));
                    s = (ch->nr[3] >> 4);
                    r = (ch->nr[3] & 0x7);
                    f = 0x40000 / (r == 0 ? 0.5 : r) / std::pow(2, (s+1));
                }

                int ls = SAMPLE_RATE * (l / 256);

                int a = ch->nr[2] >> 4;
                bool envd = (ch->nr[2] & 0x8);
                int  envp = SAMPLE_RATE * ((double)(ch->nr[2] & 0x7) / 64);

                for (int i = 0; i < ls; i++) {
                    if (ct == channel_type::square) ch->push_sample(square_generator(i, f, (double)a / 15));
                    if (ct == channel_type::noise) { ch->push_sample(noise_generator(i, f / 16, (double)a / 15)); }

                    if (ch->nr[2] & 0x7) {
                        if (!((i+1) % envp)) {
                            if (envd) { if (a < 0xf) a++; } else { if (a) a--; }
                        }
                    }
                }

                ch->buf.loadFromSamples(ch->samples.data(), ls, 1, SAMPLE_RATE);
                ch->out.play();

                ch->nr[4] &= 0x7f;
            }
        }

        void init() {
            ch1.out.setBuffer(ch1.buf);
            ch2.out.setBuffer(ch2.buf);
            ch4.out.setBuffer(ch4.buf);

            srand(time(NULL));
        }

        void write(u16 addr, u16 value, size_t size) {
            if (addr == 0xff11) ch1.nr[1] = value;
            if (addr == 0xff12) ch1.nr[2] = value;
            if (addr == 0xff13) ch1.nr[3] = value;
            if (addr == 0xff14) {ch1.nr[4] = value; trigger(&ch1); }
            if (addr == 0xff16) ch2.nr[1] = value;
            if (addr == 0xff17) ch2.nr[2] = value;
            if (addr == 0xff18) ch2.nr[3] = value;
            if (addr == 0xff19) { ch2.nr[4] = value; trigger(&ch2); }
            if (addr == 0xff20) ch4.nr[1] = value;
            if (addr == 0xff21) ch4.nr[2] = value;
            if (addr == 0xff22) ch4.nr[3] = value;
            if (addr == 0xff23) { ch4.nr[4] = value; trigger(&ch4, channel_type::noise); }
        }
    }
}