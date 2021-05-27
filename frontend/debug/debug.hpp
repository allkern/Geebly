#pragma once

// Dear ImGui: standalone example application for SDL2 + OpenGL
// (SDL is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan/Metal graphics context creation, etc.)
// (GL3W is a helper library to access OpenGL functions since there is no standard header to access modern OpenGL functions easily. Alternatives are GLEW, Glad, etc.)
// If you are new to Dear ImGui, read documentation from the docs/ folder + read the top of imgui.cpp.
// Read online: https://github.com/ocornut/imgui/tree/master/docs

#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl3.h"
#include <cstdio>
#include <thread>
#include <SDL2/SDL.h>

#include "geebly/global.hpp"
#include "lgw/color.hpp"

#include "global.hpp"
#include "ppu.hpp"
#include "cpu.hpp"
#include "spu.hpp"
#include "screen.hpp"

#include "../dialog.hpp"
#include "../input.hpp"

#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <GLES2/gl2.h>
// About Desktop OpenGL function loaders:
//  Modern desktop OpenGL doesn't have a standard portable header file to load OpenGL function pointers.
//  Helper libraries are often used for this purpose! Here we are supporting a few common ones (gl3w, glew, glad).
//  You may use another loader/header of your choice (glext, glLoadGen, etc.), or chose to manually implement your own.
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
#include <GL/gl3w.h>            // Initialize with gl3wInit()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
#include <GL/glew.h>            // Initialize with glewInit()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
#include <glad/glad.h>          // Initialize with gladLoadGL()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD2)
#include <glad/gl.h>            // Initialize with gladLoadGL(...) or gladLoaderLoadGL()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING2)
#define GLFW_INCLUDE_NONE       // GLFW including OpenGL headers causes ambiguity or multiple definition errors.
#include <glbinding/Binding.h>  // Initialize with glbinding::Binding::initialize()
#include <glbinding/gl/gl.h>
using namespace gl;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING3)
#define GLFW_INCLUDE_NONE       // GLFW including OpenGL headers causes ambiguity or multiple definition errors.
#include <glbinding/glbinding.h>// Initialize with glbinding::initialize()
#include <glbinding/gl/gl.h>
using namespace gl;
#else
#include IMGUI_IMPL_OPENGL_LOADER_CUSTOM
#endif

namespace frontend {
    namespace debug {
        namespace sdl {
            SDL_Window* window = nullptr;
            SDL_GLContext gl_context;
            ImGuiIO* io = nullptr;
        }

        bool open = false;

        bool init() {
#if defined(IMGUI_IMPL_OPENGL_ES2)
            // GL ES 2.0 + GLSL 100
            const char* glsl_version = "#version 100";
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#elif defined(__APPLE__)
            // GL 3.2 Core + GLSL 150
            const char* glsl_version = "#version 150";
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG); // Always required on Mac
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
#else
            // GL 3.0 + GLSL 130
            const char* glsl_version = "#version 130";
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#endif
            // Create sdl::window with graphics context
            SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
            SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
            SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
            SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);

            SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);

            sdl::window = SDL_CreateWindow("Debug", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1280, 720, window_flags);
            sdl::gl_context = SDL_GL_CreateContext(sdl::window);

            SDL_GL_MakeCurrent(sdl::window, sdl::gl_context);
            SDL_GL_SetSwapInterval(1); // Enable vsync

    // Initialize OpenGL loader
#if defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
            bool err = gl3wInit() != 0;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
            bool err = glewInit() != GLEW_OK;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
            bool err = gladLoadGL() == 0;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD2)
            bool err = gladLoadGL((GLADloadfunc)SDL_GL_GetProcAddress) == 0; // glad2 recommend using the windowing library loader instead of the (optionally) bundled one.
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING2)
            bool err = false;
            glbinding::Binding::initialize();
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING3)
            bool err = false;
            glbinding::initialize([](const char* name) { return (glbinding::ProcAddress)SDL_GL_GetProcAddress(name); });
#else
            bool err = false; // If you use IMGUI_IMPL_OPENGL_LOADER_CUSTOM, your loader is likely to requires some form of initialization.
#endif
            if (err) {
                fprintf(stderr, "Failed to initialize OpenGL loader!\n");
                return 1;
            }

            // Setup Dear ImGui context
            IMGUI_CHECKVERSION();
            ImGui::CreateContext();
            ImPlot::CreateContext();
            sdl::io = &ImGui::GetIO();

            // Setup Platform/Renderer backends
            ImGui_ImplSDL2_InitForOpenGL(sdl::window, sdl::gl_context);
            ImGui_ImplOpenGL3_Init(glsl_version);

            // Load Fonts
            // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
            // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
            // - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
            // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
            // - Read 'docs/FONTS.md' for more instructions and details.
            // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
            
            ImGuiIO& cio = ImGui::GetIO();
            
            cio.Fonts->Clear();
            sdl::io->Fonts->AddFontDefault();

            cio.Fonts->AddFontFromFileTTF("ubuntu-mono.ttf", 16.0f);

            ppu_panel::init();
            screen_panel::init();

            ImGui::GetStyle().WindowBorderSize = 0.0f;
            
            open = true;

            return true;
        }

        bool is_open() {
            return open;
        }

        void close() {
            open = false;

            SDL_Quit();

            // Cleanup
            ImGui_ImplOpenGL3_Shutdown();
            ImGui_ImplSDL2_Shutdown();
            ImPlot::DestroyContext();
            ImGui::DestroyContext();

            SDL_GL_DeleteContext(sdl::gl_context);
            SDL_DestroyWindow(sdl::window);
        }

        void update() {
            SDL_Event event;

            while (SDL_PollEvent(&event)) {
                ImGui_ImplSDL2_ProcessEvent(&event);

                switch (event.type) {
                    case SDL_QUIT: { close(); } break;
                    case SDL_KEYDOWN: { input::keydown_cb(event.key.keysym.sym); } break;
                    case SDL_KEYUP: { input::keyup_cb(event.key.keysym.sym); } break;
                }

                if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(sdl::window))
                    open = false;
            }

            // Start the Dear ImGui frame
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplSDL2_NewFrame(sdl::window);
            ImGui::NewFrame();

            ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);

            if (ImGui::BeginMainMenuBar()) {
                if (ImGui::BeginMenu("File")) {
                    if (ImGui::MenuItem("Open...")) {
                        std::string f = fd::open();

                        if (f.size()) {
                            _log(debug, "rom: %s", f.c_str());
                        }
                    }

                    ImGui::EndMenu();
                }

                if (ImGui::BeginMenu("GameBoy")) {
                    if (ImGui::MenuItem("Reset"));
                    if (ImGui::MenuItem("Shutdown"));

                    ImGui::EndMenu();
                }

                if (ImGui::BeginMenu("Panels")) {
                    ImGui::PushItemFlag(ImGuiItemFlags_SelectableDontClosePopup, true);

                    if (ImGui::MenuItem("CPU", nullptr, &show_cpu_panel));
                    if (ImGui::MenuItem("PPU", nullptr, &show_ppu_panel));

                    ImGui::PopItemFlag();

                    if (ImGui::BeginMenu("Screen")) {
                        ImGui::MenuItem("Show", nullptr, &show_screen_panel);
                        ImGui::SliderInt("Scale", &screen_panel::scale, 1, 4);

                        ImGui::EndMenu();
                    }

                    ImGui::EndMenu();
                }

                ImGui::EndMainMenuBar();
            }

            ImGui::ShowDemoWindow();

            //ppu_panel::render();
            if (show_cpu_panel) cpu_panel::render();
            if (show_screen_panel) screen_panel::render();
            if (show_ppu_panel) ppu_panel::render();
            spu_panel::render();

            ImGui::PopFont();

            // Rendering
            ImGui::Render();
            glViewport(0, 0, (int)sdl::io->DisplaySize.x, (int)sdl::io->DisplaySize.y);
            glClearColor(0.1, 0.1, 0.1, 1);
            glClear(GL_COLOR_BUFFER_BIT);
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
            SDL_GL_SwapWindow(sdl::window);
        }
        
        void update_proc() {
            debug::init();

            while (debug::is_open()) debug::update();

            debug::close();
        }

        void start() {
            std::thread debug_thread(update_proc);

            debug_thread.detach();

            _log(info, "Waiting for debug to fully initialize...");

            std::this_thread::sleep_for(std::chrono::milliseconds(100));

            _log(ok, "Initialized debug");
        }
    }
}