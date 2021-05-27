#pragma once

#include "SDL2/SDL_audio.h"
#include "SDL2/SDL.h"

#define AUDIO_DEVICE_SAMPLERATE 48000
#define AUDIO_BUFFER_SIZE 32
#define AUDIO_RESAMPLER_FILL_SIZE \
    ((size_t)(AUDIO_BUFFER_SIZE * ((double)SPU_NATIVE_SAMPLERATE / (double)AUDIO_DEVICE_SAMPLERATE)))

#include <cstdint>
#include <vector>
#include <array>

#include "imgui.h"

namespace frontend {
    namespace audio {
        using namespace gameboy;

        SDL_AudioDeviceID dev;
        SDL_AudioSpec obtained, desired;

        SDL_AudioStream* stream = nullptr;

        std::array <int16_t, AUDIO_RESAMPLER_FILL_SIZE * SPU_CHANNELS> mixed_sample_buf = { 0 };

        void update(void* ud, uint8_t* buf, int size) {
            for (size_t i = 0; i < (AUDIO_RESAMPLER_FILL_SIZE * 2); i += 2)
                *reinterpret_cast<int32_t*>(&mixed_sample_buf[i]) = spu::get_sample();

            SDL_AudioStreamPut(stream, mixed_sample_buf.data(), (mixed_sample_buf.size()) << 1);

            SDL_AudioStreamFlush(stream);

            SDL_AudioStreamGet(stream, buf, size);
        }

        void init() {
            if (!sound_disabled) {
                SDL_Init(SDL_INIT_AUDIO);

                stream = SDL_NewAudioStream(
                    AUDIO_S16SYS,
                    SPU_CHANNELS,
                    SPU_NATIVE_SAMPLERATE,
                    AUDIO_S16SYS,
                    SPU_CHANNELS,
                    AUDIO_DEVICE_SAMPLERATE
                );

                desired.freq     = AUDIO_DEVICE_SAMPLERATE;
                desired.format   = AUDIO_S16SYS;
                desired.channels = SPU_CHANNELS;
                desired.samples  = AUDIO_BUFFER_SIZE;
                desired.callback = &update;
                desired.userdata = nullptr;

                dev = SDL_OpenAudioDevice(nullptr, 0, &desired, nullptr, 0);

                if (dev) SDL_PauseAudioDevice(dev, 0);
            }
        }
    }
}