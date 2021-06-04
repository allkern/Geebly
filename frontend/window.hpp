#pragma once

#ifdef _WIN32
#define GEEBLY_NATIVE_VSYNC
#define GEEBLY_SDL_WINDOW_FLAGS SDL_WINDOW_VULKAN
#endif

#ifdef __linux__
#ifdef GEEBLY_NATIVE_VSYNC
#undef GEEBLY_NATIVE_VSYNC
#endif
#define GEEBLY_SDL_WINDOW_FLAGS SDL_WINDOW_OPENGL
#endif

#include "SDL.h"

#include <chrono>
#include <unordered_map>

#define PPU_WIDTH  160
#define PPU_HEIGHT 144

#define STR1(m) #m
#define STR(m) STR1(m)

namespace gameboy {
    namespace screen {
        namespace sdl {
            SDL_Window* window = nullptr;
            SDL_Renderer* renderer = nullptr;
            SDL_Texture* texture = nullptr;
        }

        typedef void (*key_event_callback_t) (SDL_Keycode);
        typedef void (*rom_drop_callback_t) (const char*);

        key_event_callback_t keydown_cb, keyup_cb;
        rom_drop_callback_t rom_drop_cb;

        // FPS tracking stuff
        auto start = std::chrono::high_resolution_clock::now(), end = start;
        size_t frames_rendered = 0, fps = 0;

        inline void register_keydown_cb(const key_event_callback_t& kd) {
            keydown_cb = kd;
        }

        inline void register_keyup_cb(const key_event_callback_t& ku) {
            keyup_cb = ku;
        }

        // To-do: Make ROM dropping better
        inline void register_rom_dropped_cb(const rom_drop_callback_t& rd) {
            rom_drop_cb = rd;
        }

        bool open = true;

        bool is_open() {
            return open;
        }

        size_t get_fps() {
            return fps;
        }

        bool fps_ready() {
            end = std::chrono::high_resolution_clock::now();

            std::chrono::duration <double> d = end - start;

            return std::chrono::duration_cast<std::chrono::seconds>(d).count() == 1;
        }

        void close() {
            open = false;
            
            SDL_DestroyTexture(sdl::texture);
            SDL_DestroyRenderer(sdl::renderer);
            SDL_DestroyWindow(sdl::window);

            SDL_Quit();
        }

        void init(size_t scale) {
            uint32_t SDL_INIT_FLAGS = SDL_INIT_VIDEO | SDL_INIT_EVENTS;

            if (!sound_disabled) SDL_INIT_FLAGS |= SDL_INIT_AUDIO;

            SDL_Init(SDL_INIT_FLAGS);

            sdl::window = SDL_CreateWindow(
                "Geebly " STR(GEEBLY_VERSION_TAG) " " STR(GEEBLY_COMMIT_HASH),
                SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                PPU_WIDTH * scale, PPU_HEIGHT * scale,
                GEEBLY_SDL_WINDOW_FLAGS
            );

            sdl::renderer = SDL_CreateRenderer(
                sdl::window,
                -1,
                SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
            );

            sdl::texture = SDL_CreateTexture(
                sdl::renderer,
                SDL_PIXELFORMAT_RGBA8888,
                SDL_TEXTUREACCESS_STREAMING,
                PPU_WIDTH, PPU_HEIGHT
            );
        }

        void update(uint32_t* buf) {
            end = std::chrono::high_resolution_clock::now();

            std::chrono::duration <double> d = end - start;

            if (std::chrono::duration_cast<std::chrono::seconds>(d).count() == 1) {
                fps = frames_rendered;
                frames_rendered = 0;
                start = std::chrono::high_resolution_clock::now();
                end = start;
            }

#ifndef GEEBLY_NATIVE_VSYNC
            int start = SDL_GetTicks();
#endif

            SDL_RenderClear(sdl::renderer);

            SDL_UpdateTexture(
                sdl::texture,
                NULL,
                buf,
                PPU_WIDTH * sizeof(uint32_t)
            );

            SDL_RenderCopy(sdl::renderer, sdl::texture, NULL, NULL);

            SDL_RenderPresent(sdl::renderer);

            frames_rendered++;

            SDL_Event event;

            while (SDL_PollEvent(&event)) {
                switch (event.type) {
                    case SDL_DROPFILE: { rom_drop_cb(event.drop.file); } break;
                    case SDL_QUIT: { close(); } break;
                    case SDL_KEYDOWN: { keydown_cb(event.key.keysym.sym); } break;
                    case SDL_KEYUP: { keyup_cb(event.key.keysym.sym); } break;
                }
            }

            // Shitty implementation of Vsync that doesn't actually Vsync.
            // It just tries to keep a framerate close to 60 Hz
            // Thankfully/disgracefully, this is probably only present on Linux hosts where
            // there isn't native Vsync (X11).
#ifndef GEEBLY_NATIVE_VSYNC
            int time = SDL_GetTicks() - start;

            if (time < 0) return;

            int delay = 14 - time;

            if (delay > 0) {
                SDL_Delay(delay);
            }
#endif
        }
    }
}