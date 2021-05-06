#pragma once

#include "../../aliases.hpp"
#include "../../global.hpp"
#include "../../log.hpp"

#include "memory.hpp"
#include "square.hpp"
#include "noise.hpp"
#include "wave.hpp"

namespace gameboy {
    namespace spu {
        std::array <int16_t, SPU_RESAMPLER_FILL_SIZE * 2> mixed_sample_buf = { 0 };

        int32_t pan(int16_t sample, size_t channel) {
            int32_t panned = 0x0;

            if (nr[15] & (0x1 << (channel - 1))) panned |= sample;
            if (nr[15] & (0x10 << (channel - 1))) panned |= (sample << 16);

            return panned;
        }

        int16_t left, right = 0x0;

        void audio_update_cb(void* ud, u8* buf, int size) {
            for (size_t i = 0; i < (SPU_RESAMPLER_FILL_SIZE * 2); i += 2) {
                int16_t s1 = ch1.get_sample(),
                        s2 = ch2.get_sample(),
                        s3 = ch3.get_sample(),
                        s4 = ch4.get_sample(),
                        ls1 = s1,
                        ls2 = s2,
                        ls3 = s3,
                        ls4 = s4;

                if (stereo) {
                    if (!(nr[0x15] & 0x10)) ls1 = 0;
                    if (!(nr[0x15] & 0x20)) ls2 = 0;
                    if (!(nr[0x15] & 0x40)) ls3 = 0;
                    if (!(nr[0x15] & 0x80)) ls4 = 0;

                    mixed_sample_buf[i] = (((ls1 + ls2 + ls3 + ls4) / 4) * so1_output_level) * master_volume;

                    ls1 = s1;
                    ls2 = s2;
                    ls3 = s3;
                    ls4 = s4;

                    if (!(nr[0x15] & 0x1)) ls1 = 0;
                    if (!(nr[0x15] & 0x2)) ls2 = 0;
                    if (!(nr[0x15] & 0x4)) ls3 = 0;
                    if (!(nr[0x15] & 0x8)) ls4 = 0;

                    mixed_sample_buf[i+1] = (((ls1 + ls2 + ls3 + ls4) / 4) * so2_output_level) * master_volume;
                } else {
                    mixed_sample_buf[i] = ((ls1 + ls2 + ls3 + ls4) / 4);
                    mixed_sample_buf[i+1] = mixed_sample_buf[i];
                }
            }

            SDL_AudioStreamPut(stream, mixed_sample_buf.data(), (mixed_sample_buf.size()) << 1);

            SDL_AudioStreamFlush(stream);

            SDL_AudioStreamGet(stream, buf, size);
        }

        inline int16_t mix(int16_t a, int16_t b) {
            return ((a < 0) && (b < 0)) ? ((a + b) - ((a * b) / (std::min(a, b)))) :
                   ((a > 0) && (b > 0)) ? ((a + b) - ((a * b) / (std::max(a, b)))) :
                   (a + b);
        }

        void init() {
            stream = SDL_NewAudioStream(
                AUDIO_S16SYS,
                2,
                SPU_NATIVE_SAMPLERATE,
                AUDIO_S16SYS,
                2,
                SPU_DEVICE_SAMPLERATE
            );

            desired.freq     = SPU_DEVICE_SAMPLERATE;
            desired.format   = AUDIO_S16SYS;
            desired.channels = 2;
            desired.samples  = SPU_BUFFER_SIZE;
            desired.callback = &audio_update_cb;
            desired.userdata = nullptr;

            or_mask[0x00] = 0x80;
            or_mask[0x01] = 0x3f;
            or_mask[0x04] = 0xb8;
            or_mask[0x05] = 0xff;
            or_mask[0x06] = 0x3f;
            or_mask[0x09] = 0xb8;
            or_mask[0x0a] = 0x7f;
            or_mask[0x0b] = 0xff;
            or_mask[0x0c] = 0x9f;
            or_mask[0x0e] = 0xb8;
            or_mask[0x0f] = 0xff;
            or_mask[0x10] = 0xff;
            or_mask[0x13] = 0xbf;
            or_mask[0x16] = 0x70;

            dev = SDL_OpenAudioDevice(nullptr, 0, &desired, nullptr, 0);

            ch1.init(nr[0x0]);
            ch2.init(nr[0x5]);
            ch3.init(nr[0xa]);
            ch4.init(nr[0xf]);

            if (dev) SDL_PauseAudioDevice(dev, 0);
        }

        void reset(bool reset_on = true) {
            if (reset_on) spu_on = ~spu_on;

            ch1.reset();
            ch2.reset();
            ch3.reset();
            ch4.reset();
        }

        void write(u16 addr, u16 value, size_t size) {
            // Handle a write to Wave RAM
            if (addr >= 0xff30 && addr <= 0xff3f) {
                utility::default_mb_write(wave_ram.data(), addr, value, size, SPU_BEGIN+0x20);
                return;
            }

            if ((addr >= 0xff10) && (addr <= 0xff25) && (!(nr[0x16] & 0x80))) return;

            utility::default_mb_write(nr.data(), addr, value, size, SPU_BEGIN);

            if (sound_disabled) return;
    
            if (addr == 0xff14) ch1.update_state();
            if (addr == 0xff19) ch2.update_state();
            if (addr == 0xff1e) ch3.update_state();
            if (addr == 0xff23) ch4.update_state();

            // To-do: Improve ch3 output level emulation
            //        Overall better SPU accuracy needed (using DIV as timing source?)
            if (addr == 0xff1c) ch3.update_output_level();

            if (addr == 0xff24) {
                so1_output_level = (double)(nr[0x14] & 0x7) / 7;
                so2_output_level = (double)((nr[0x14] >> 4) & 0x7) / 7;
            }

            if (addr == 0xff26) {
                if ((!(value & 0x80) && (nr[0x16] & 0x80)) || ((value & 0x80) && (!(nr[0x16] & 0x80)))) reset();

                nr[0x16] = value;
            }
        }

        u32 read(u16 addr, size_t size) {
            if (addr >= 0xff10 && addr <= 0xff26) {
                return utility::default_mb_read(nr.data(), addr, size, SPU_BEGIN) | or_mask.at(addr - SPU_BEGIN);
            }

            if (addr >= 0xff30 && addr <= 0xff3f) {
                return utility::default_mb_read(wave_ram.data(), addr, size, SPU_BEGIN+0x20);
            }

            return 0xff;
        }
    }
}