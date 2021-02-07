#pragma once

#ifdef _WIN32
    #include "SDL_audio.h"
#endif

#ifdef __linux__
    #include "SDL2/SDL_audio.h"
#endif

#include "../aliases.hpp"
#include "../global.hpp"
#include "../log.hpp"
#include "../cpu/registers.hpp"

#include <iostream>
#include <ctime>
#include <cmath>
#include <array>

#define SPU_SAMPLERATE 48000
#define SPU_BEGIN 0xff10
#define SPU_END 0xff3f

#define SPUNR_SWPC 0x0
#define SPUNR_LENC 0x1
#define SPUNR_ENVC 0x2
#define SPUNR_FREQ 0x3
#define SPUNR_CTRL 0x4

#define CTRL_RESTR 0x80
#define CTRL_LENCT 0x40
#define CTRL_FREQH 0x7

#define ENVC_STVOL 0xf0
#define ENVC_DIRCT 0x8
#define ENVC_ENVSN 0x7

#define LENC_WDUTY 0xc0
#define LENC_LENCT 0x3f

namespace gameboy {
    namespace spu {
        namespace detail {
            template <typename T> inline int sign(T val) {
                return (T(0) < val) - (val < T(0));
            }
        }
        typedef std::array <u8, 0x17> nr_t;

        struct channel_t {
            virtual void init(u8&) = 0;
            virtual void update() {};
            virtual double generate() { return 0.0; };
        };

        typedef std::array <channel_t*, 0x4> channels_t;

        SDL_AudioSpec obtained, desired;

        auto stream = SDL_NewAudioStream(
            AUDIO_S16,
            1,
            SPU_SAMPLERATE,
            AUDIO_F32,
            2,
            SPU_SAMPLERATE
        );

        int16_t generate_square_sample(double t, double f, double a) {
            t /= SPU_SAMPLERATE;
            return detail::sign(std::sin((2.0 * M_PI) * f * t)) * 0x7fff;
        }    

        double clk = 0.0;

        nr_t nr = {
            0x80, 0xbf, 0xf3, 0xff, 0x3f, 0xff,
            0x3f, 0x00, 0xff, 0xbf,
            0x7f, 0xff, 0x9f, 0xff, 0xbf, 0xff,
            0xff, 0x00, 0x00, 0xbf, 0x77,
            0xf3, 0xf1
        };

        #define TEST_REG(r, m) (nr[r] & m)
        #define RESET_REG(r, m) nr[r] &= (~m)

        struct square_t : public channel_t {
            u8* nr = nullptr;

            bool playing = false, fired = false;

            void update() {
                if (!fired) {
                    playing = TEST_REG(SPUNR_CTRL, CTRL_RESTR);
                    fired = playing;
                }

                if (playing) {
                    generate();
                    RESET_REG(SPUNR_CTRL, CTRL_RESTR);
                }
            }

            double generate() override {
                double local_clk = clk;

                if (playing) {
                    double f = 0x20000 / (0x1000 - (nr[SPUNR_FREQ] | ((nr[SPUNR_CTRL] & CTRL_FREQH) << 8)));
                    if (TEST_REG(SPUNR_CTRL, CTRL_LENCT)) {
                    
                    } else {
                        for (size_t i = 0; i < desired.samples * 10; i++) {
                            int16_t sample = generate_square_sample(local_clk++, f, 1);
                            SDL_AudioStreamPut(stream, &sample, sizeof(int16_t));
                        }
                    }
                }

                return local_clk;
            }

            void init(u8& nr) override {
                this->nr = &nr;
            }
        } ch2;

        // struct noise_t : public channel_t {
        //     u8* nr = nullptr;

        //     void init(u8& nr) override {
        //         this->nr = &nr;
        //     }
        // };

        // struct wave_t : public channel_t {
        //     u8* nr = nullptr;

        //     void init(u8& nr) override {
        //         this->nr = &nr;
        //     }
        // };

        //channels_t ch = {
        //    new square_t(),
        //    new square_t(),
        //    new wave_t(),
        //    new _t(),
        //};
 
        static void audio_cb(void* ud, uint8_t* buf, int size) {
            clk = ch2.generate();
            SDL_AudioStreamGet(stream, buf, size);
            SDL_AudioStreamFlush(stream);
        }

        void update() {
            ch2.update();
        }

        void init() {
            ch2.init(nr[0x0]);
            //ch1.init(nr[0x5]);
            //ch1.init(nr[0xa]);
            //ch1.init(nr[0xf]);

            desired.freq        = SPU_SAMPLERATE;
            desired.format      = AUDIO_F32;
            desired.channels    = 1;
            desired.samples     = 0x800;
            desired.callback    = &audio_cb;
            desired.userdata    = NULL;

            auto dev = SDL_OpenAudioDevice(nullptr, 0, &desired, &obtained, SDL_AUDIO_ALLOW_ANY_CHANGE);

            if (dev) {
                std::cout << "desired:\n" <<
                    "\tfreq    : " << desired.freq     << "\n" <<
                    "\tformat  : " << std::hex << desired.format << "\n" <<
                    "\tchannels: " << std::dec << (uint16_t)desired.channels << "\n" <<
                    "\tsamples : " << std::dec << desired.samples  << "\n" <<
                    "\tcallback: " << desired.callback << "\n" <<
                    "\tuserdata: " << desired.userdata << "\n\n";

                std::cout << "obtained:\n" <<
                    "\tfreq    : " << std::dec << obtained.freq << "\n" <<
                    "\tformat  : " << std::hex << obtained.format << "\n" <<
                    "\tchannels: " << std::dec << (uint16_t)obtained.channels << "\n" <<
                    "\tsamples : " << std::dec << obtained.samples  << "\n" <<
                    "\tcallback: " << obtained.callback << "\n" <<
                    "\tuserdata: " << obtained.userdata << "\n";
                SDL_PauseAudioDevice(dev, 0);
            } else {
                std::cout << "Failed to open the desired audio device\n";
            }
        }

        void write(u16 addr, u16 value, size_t size) {
            _log(debug, "addr=%04x, value=%02x", addr, value);
            utility::default_mb_write(nr.data(), addr, value, size, SPU_BEGIN);
        }

        u32 read(u16 addr, size_t size) {
            return utility::default_mb_read(nr.data(), addr, size, SPU_BEGIN);
        }
    }
}