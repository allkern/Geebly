#pragma once

#ifdef _WIN32
#include "SDL.h"
#endif

#ifdef __linux__
#include "SDL2/SDL.h"
#endif

#define LGW_FORMAT_ARGB8888

#include "lgw/framebuffer.hpp"

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

        // FPS tracking stuff
        auto start = std::chrono::high_resolution_clock::now();
        size_t frames_rendered = 0, fps = 0;

        typedef lgw::framebuffer <PPU_WIDTH, PPU_HEIGHT> framebuffer_t;

        framebuffer_t* frame = nullptr;

        void init(framebuffer_t* f_ptr, size_t window_scale) {
            frame = f_ptr;

            sdl::window = SDL_CreateWindow(
                "Accurate PPU Test",
                SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                PPU_WIDTH * window_scale, PPU_HEIGHT * window_scale,
                SDL_WINDOW_OPENGL
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

        bool open = true;

        inline bool is_open() {
            return open;
        }

        inline size_t get_fps() {
            return fps;
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

            if (SDL_PollEvent(&event)) {
                switch (event.type) {
                    case SDL_QUIT: { open = false; } break;
                }
            }

            frame->clear();
        }

        void close() {
            SDL_DestroyTexture(sdl::texture);
            SDL_DestroyRenderer(sdl::renderer);
            SDL_DestroyWindow(sdl::window);

            SDL_Quit();
        }
    }
}