#pragma once

#include "SDL2/SDL.h"

#include "imgui.h"
#include "imgui_sdl.h"
#include "implot.h"

#define GEEBLY_DEBUGGER_WIDTH 500
#define GEEBLY_DEBUGGER_HEIGHT 500

namespace gameboy {
    namespace debug {
        namespace sdl {
            SDL_Window* window = nullptr;
            SDL_Renderer* renderer = nullptr;
            SDL_Texture* texture = nullptr;
            int window_id = 0;
        }

        bool open = false;

        size_t update_counter = 10000, events_handle_counter = 100;

        void init() {
            SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS);

            sdl::window = SDL_CreateWindow(
                "Debugger",
                25, 25,
                GEEBLY_DEBUGGER_WIDTH, GEEBLY_DEBUGGER_HEIGHT,
                SDL_WINDOW_VULKAN
            );

            sdl::window_id = SDL_GetWindowID(sdl::window);

            sdl::renderer = SDL_CreateRenderer(sdl::window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

            ImGui::CreateContext();
            ImPlot::CreateContext();

            ImGuiSDL::Initialize(sdl::renderer, GEEBLY_DEBUGGER_WIDTH, GEEBLY_DEBUGGER_HEIGHT);

            open = true;
        }

        bool is_open() {
            return open;
        }

        void close() {
            ImGuiSDL::Deinitialize();

            SDL_DestroyRenderer(sdl::renderer);
            SDL_DestroyWindow(sdl::window);

            ImPlot::DestroyContext();
            ImGui::DestroyContext();
        }

        void handle_events() {
            SDL_Event event;

            while (SDL_PollEvent(&event)) {
                if (event.window.windowID != sdl::window_id) return;

                switch (event.type) {
                    case SDL_QUIT: { open = false; close(); } break;
                }
            }
        }

        void update() {
            if (update_counter--) {
                return;
            } else {
                update_counter = 10000;
            }

            ImGuiIO& io = ImGui::GetIO();

            handle_events();

            int mouse_x, mouse_y, wheel;
		    const int buttons = SDL_GetMouseState(&mouse_x, &mouse_y);

            io.DeltaTime = 1.0f / 60.0f;
            io.MousePos = ImVec2(static_cast<float>(mouse_x), static_cast<float>(mouse_y));
            io.MouseDown[0] = buttons & SDL_BUTTON(SDL_BUTTON_LEFT);
            io.MouseDown[1] = buttons & SDL_BUTTON(SDL_BUTTON_RIGHT);
            io.MouseWheel = static_cast<float>(wheel);

            SDL_SetRenderDrawColor(sdl::renderer, 255, 255, 255, 255);
            SDL_RenderClear(sdl::renderer);
            ImGui::NewFrame();

            ImGui::SetNextWindowSize(ImVec2(500, 500));
            ImGui::SetNextWindowPos(ImVec2(0, 0));

            if (ImGui::Begin("Slider", nullptr,
                ImGuiWindowFlags_NoCollapse |
                ImGuiWindowFlags_NoResize |
                ImGuiWindowFlags_NoTitleBar |
                ImGuiWindowFlags_NoMove)) {
                ImGui::SliderFloat("Master Volume", &master_volume, 0.0f, 1.0f);
                ImGui::End();
            }

            ImGui::Render();
            ImGuiSDL::Render(ImGui::GetDrawData());

            SDL_RenderPresent(sdl::renderer);
        }
    }
}