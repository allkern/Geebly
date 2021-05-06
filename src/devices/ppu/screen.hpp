#pragma once

#ifdef _WIN32
#include "SDL2/SDL.h"
#define GEEBLY_SDL_WINDOW_FLAGS SDL_WINDOW_VULKAN
#endif

#ifdef __linux__
#include "SDL2/SDL.h"
#define GEEBLY_SDL_WINDOW_FLAGS SDL_WINDOW_OPENGL
#endif

#define LGW_FORMAT_ARGB8888

#include "lgw/framebuffer.hpp"

#include <functional>
#include <chrono>

#define PPU_WIDTH  160
#define PPU_HEIGHT 144

namespace gameboy {
    namespace screen {
        namespace sdl {
            SDL_Window* window = nullptr;
            SDL_Renderer* renderer = nullptr;
            SDL_Texture* texture = nullptr;
        }

        typedef std::function<void(SDL_Keycode)> key_event_callback_t;
        typedef std::function<void(std::string)> rom_drop_callback_t;

        key_event_callback_t keydown_cb, keyup_cb;
        rom_drop_callback_t rom_drop_cb;

        // FPS tracking stuff
        auto start = std::chrono::high_resolution_clock::now();
        size_t frames_rendered = 0, fps = 0;

        typedef lgw::framebuffer <PPU_WIDTH, PPU_HEIGHT> framebuffer_t;

        framebuffer_t* frame = nullptr;

        void init(framebuffer_t* f_ptr, size_t window_scale) {
            SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS);
            frame = f_ptr;

            sdl::window = SDL_CreateWindow(
                "Geebly",
                SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                PPU_WIDTH * window_scale, PPU_HEIGHT * window_scale,
                GEEBLY_SDL_WINDOW_FLAGS
            );

            sdl::renderer = SDL_CreateRenderer(
                sdl::window,
                -1,
                SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
            );

            sdl::texture = SDL_CreateTexture(
                sdl::renderer,
                SDL_PIXELFORMAT_ARGB8888,
                SDL_TEXTUREACCESS_STREAMING,
                PPU_WIDTH, PPU_HEIGHT
            );
        }

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

        inline bool is_open() {
            return open;
        }

        inline size_t get_fps() {
            return fps;
        }

        void close() {
            open = false;
            
            SDL_DestroyTexture(sdl::texture);
            SDL_DestroyRenderer(sdl::renderer);
            SDL_DestroyWindow(sdl::window);

            SDL_Quit();
        }

        void update() {
            auto end = std::chrono::high_resolution_clock::now();

            std::chrono::duration <double> d = end - start;

            if (std::chrono::duration_cast<std::chrono::seconds>(d).count() >= 1) {
                fps = frames_rendered;
                frames_rendered = 0;
                start = std::chrono::high_resolution_clock::now();
            }

            SDL_RenderClear(sdl::renderer);

            SDL_UpdateTexture(
                sdl::texture,
                NULL,
                frame->get_buffer(),
                PPU_WIDTH * sizeof(uint32_t)
            );

            SDL_RenderCopy(sdl::renderer, sdl::texture, NULL, NULL);

            SDL_RenderPresent(sdl::renderer);

            frames_rendered++;

            SDL_Event event;

            while (SDL_PollEvent(&event)) {
                switch (event.type) {
                    case SDL_DROPFILE: { rom_drop_cb(std::string(event.drop.file)); } break;
                    case SDL_QUIT: { close(); } break;
                    case SDL_KEYDOWN: { keydown_cb(event.key.keysym.sym); } break;
                    case SDL_KEYUP: { keyup_cb(event.key.keysym.sym); } break;
                }
            }

            frame->clear();
        }
    }
}