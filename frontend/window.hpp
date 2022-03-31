#pragma once

#ifdef _WIN32
#define GEEBLY_NATIVE_VSYNC
#define GEEBLY_SDL_WINDOW_FLAGS (SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI)
#include "dwmapi.h"
#include "SDL_syswm.h"
#include "dialog.hpp"
#endif

#ifdef __linux__
#ifdef GEEBLY_NATIVE_VSYNC
#undef GEEBLY_NATIVE_VSYNC
#endif
#define GEEBLY_SDL_WINDOW_FLAGS SDL_WINDOW_OPENGL
#endif

#include "SDL.h"
#include "SDL_ttf.h"

#include "ui/ui.hpp"

#ifndef STBI_INCLUDE_STB_IMAGE_H
#define STB_IMAGE_IMPLEMENTATION
#include "../../stb_image.h"
#endif

#include "SDL_shader.h"

#include <chrono>
#include <unordered_map>

#define PPU_WIDTH  160
#define PPU_HEIGHT 144

#define STR1(m) #m
#define STR(m) STR1(m)

std::vector <uint32_t> scaled;

void scale_and_flip_buf(uint32_t* buf, int scale) {
    size_t size = (PPU_WIDTH * scale) * (PPU_HEIGHT * scale);

    if (scaled.size() < size)
        scaled.resize(size);

    for (int y = (PPU_HEIGHT - 1); y != 0; y--) {
        for (int x = 0; x < PPU_WIDTH; x++) {
            uint32_t color = buf[x + (y * PPU_WIDTH)];
            uint32_t reordered =
                (((color >> 0) & 0xff) << 24) |
                (((color >> 8) & 0xff) << 16) |
                (((color >> 16) & 0xff) << 8) |
                (((color >> 24) & 0xff) << 0);
                

            int bx = x * scale,
                by = (PPU_HEIGHT - 1 - y) * scale;
            
            for (int iy = 0; iy < scale; iy++)
                for (int ix = 0; ix < scale; ix++)
                    scaled.at((bx + ix) + ((by + iy) * (PPU_WIDTH * scale))) = reordered;
        }
    }
}

namespace frontend {
    bool ntsc_codec_enabled = false,
         start_with_gui = false,
         blend_frames = false;

    namespace window {
        namespace sdl {
            SDL_Window* window = nullptr;
            SDL_Renderer* renderer = nullptr;
            SDL_Texture* texture = nullptr;
        }

        bool shader_stack_available = true;

        SDL_Shader* encoder = nullptr,
                  * decoder = nullptr,
                  * hermite = nullptr,
                  * process = nullptr,
                  * curvature = nullptr;

        std::array <std::vector <uint32_t>, 5> buffers;

        bool font_present;

        void load_icon() {
            SDL_Surface* surface;

            int req_format = STBI_rgb_alpha;
            int width, height, orig_format;
            unsigned char* data = stbi_load("icon.png", &width, &height, &orig_format, req_format);

            Uint32 rmask, gmask, bmask, amask;

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
            int shift = (req_format == STBI_rgb) ? 8 : 0;
            rmask = 0xff000000 >> shift;
            gmask = 0x00ff0000 >> shift;
            bmask = 0x0000ff00 >> shift;
            amask = 0x000000ff >> shift;
#else // little endian, like x86
            rmask = 0x000000ff;
            gmask = 0x0000ff00;
            bmask = 0x00ff0000;
            amask = (req_format == STBI_rgb) ? 0 : 0xff000000;
#endif
            int depth, pitch;

            if (req_format == STBI_rgb) {
                depth = 24;
                pitch = 3*width; // 3 bytes per pixel * pixels per row
            } else { // STBI_rgb_alpha (RGBA)
                depth = 32;
                pitch = 4*width;
            }

            surface = SDL_CreateRGBSurfaceFrom((void*)data, width, height, depth, pitch, rmask, gmask, bmask, amask);

            // The icon is attached to the window pointer
            SDL_SetWindowIcon(sdl::window, surface);

            // ...and the surface containing the icon pixel data is no longer required.
            SDL_FreeSurface(surface);
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

        int window_scale = 0;

        void init(int scale) {
            uint32_t SDL_INIT_FLAGS = SDL_INIT_VIDEO | SDL_INIT_EVENTS;

            if (!gameboy::sound_disabled) SDL_INIT_FLAGS |= SDL_INIT_AUDIO;

            SDL_Init(SDL_INIT_FLAGS);

            if (start_with_gui) scale = 3;

            window_scale = scale;

            sdl::window = SDL_CreateWindow(
                "Geebly " STR(GEEBLY_VERSION_TAG) " " STR(GEEBLY_COMMIT_HASH),
                SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                PPU_WIDTH * scale, PPU_HEIGHT * scale,
                GEEBLY_SDL_WINDOW_FLAGS
            );

            if (ntsc_codec_enabled) {
                //SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengl");
            }

            SDL_GL_CreateContext(sdl::window);

            sdl::renderer = SDL_CreateRenderer(
                sdl::window,
                -1,
                SDL_RENDERER_PRESENTVSYNC
            );

            sdl::texture = SDL_CreateTexture(
                sdl::renderer,
                SDL_PIXELFORMAT_RGBA8888,
                SDL_TEXTUREACCESS_STREAMING,
                PPU_WIDTH, PPU_HEIGHT
            );

            SDL_SetRenderDrawBlendMode(sdl::renderer, SDL_BLENDMODE_BLEND);

            gl3wInit();

            if (!ntsc_codec_enabled) goto continue_without_codec;

            encoder = SDL_CreateShader(sdl::window, "shaders/identity.vert", "shaders/encode.frag");
            decoder = SDL_CreateShader(sdl::window, "shaders/identity.vert", "shaders/decode.frag");
            hermite = SDL_CreateShader(sdl::window, "shaders/identity.vert", "shaders/hermite.frag");
            process = SDL_CreateShader(sdl::window, "shaders/identity.vert", "shaders/process.frag");
            curvature = SDL_CreateShader(sdl::window, "shaders/identity.vert", "shaders/curvature.frag");

            if (!encoder) { _log(warning, "%s", SDL_GetError()); shader_stack_available = false; goto unavailable; }
            if (!decoder) { _log(warning, "%s", SDL_GetError()); shader_stack_available = false; goto unavailable; }
            if (!hermite) { _log(warning, "%s", SDL_GetError()); shader_stack_available = false; goto unavailable; }
            if (!process) { _log(warning, "%s", SDL_GetError()); shader_stack_available = false; goto unavailable; }
            if (!curvature) { _log(warning, "%s", SDL_GetError()); shader_stack_available = false; goto unavailable; }

            unavailable:
            if (!shader_stack_available) {
                _log(warning, "Shader stack unavailable, continuing with default rendering");

                return;
            }

            for (std::vector <uint32_t>& b : buffers)
                b.resize((PPU_WIDTH * window_scale) * (PPU_HEIGHT * window_scale));

            SDL_ShaderBindRawTexture(encoder, nullptr, PPU_WIDTH * window_scale, PPU_HEIGHT * window_scale);
            SDL_ShaderBindRawTexture(decoder, buffers[0].data(), PPU_WIDTH * window_scale, PPU_HEIGHT * window_scale);
            SDL_ShaderBindRawTexture(hermite, buffers[1].data(), PPU_WIDTH * window_scale, PPU_HEIGHT * window_scale);
            SDL_ShaderBindRawTexture(process, buffers[2].data(), PPU_WIDTH * window_scale, PPU_HEIGHT * window_scale);
            SDL_ShaderBindRawTexture(curvature, buffers[3].data(), PPU_WIDTH * window_scale, PPU_HEIGHT * window_scale);

            if (!SDL_CreateShaderFramebuffer(encoder)) { _log(debug, "encoder: %s", SDL_GetError()); }
            if (!SDL_CreateShaderFramebuffer(decoder)) { _log(debug, "decoder: %s", SDL_GetError()); }
            if (!SDL_CreateShaderFramebuffer(hermite)) { _log(debug, "hermite: %s", SDL_GetError()); }
            if (!SDL_CreateShaderFramebuffer(process)) { _log(debug, "process: %s", SDL_GetError()); }

            if (ntsc_codec_enabled)
                SDL_GL_SetSwapInterval(1);

            continue_without_codec:
            
            ui::init(sdl::window, sdl::renderer, sdl::texture, open);
            ui::push_font("ubuntu-mono.ttf", 24);
            ui::push_font("arial.ttf", 24);
            ui::push_font("roboto.ttf", 24);
            ui::load_main_menu();

            if (start_with_gui) ui::show();
        }

#ifdef _WIN32
        HWND get_hwnd() {
            SDL_SysWMinfo wmi;
            SDL_VERSION(&wmi.version);
            SDL_GetWindowWMInfo(sdl::window, &wmi);

            return wmi.info.win.window;
        }
#endif
        float time = 0.0;
        int decoder_fir_size = 4;

        std::array <uint32_t, 160 * 144> blended;

        void blend_frames(uint32_t* current, uint32_t* prev) {
            for (int y = 0; y < 144; y++) {
                for (int x = 0; x < 160; x++) {
                    size_t offset = x + (y * 160);

                    uint32_t cp = current[offset],
                             pp = prev[offset];

                    int rc = ((cp >> 24) & 0xff),
                        gc = ((cp >> 16) & 0xff),
                        bc = ((cp >> 8 ) & 0xff),
                        rp = ((pp >> 24) & 0xff),
                        gp = ((pp >> 16) & 0xff),
                        bp = ((pp >> 8 ) & 0xff),
                        rr = (rc + rp) / 2,
                        gr = (gc + gp) / 2,
                        br = (bc + bp) / 2;
                            
                    blended[offset] = (rr << 24) | (gr << 16) | (br << 8) | 0xff;
                }
            }
        }

        bool lctrl_down = false;

        void update(uint32_t* current_frame, uint32_t* prev_frame) {
            uint32_t* buf = ntsc_codec_enabled || !blend_frames ? current_frame : blended.data();

            if (!ntsc_codec_enabled && blend_frames)
                blend_frames(current_frame, prev_frame);

            ui::update(buf);

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

            if (shader_stack_available && ntsc_codec_enabled) {
                scale_and_flip_buf(buf, window_scale);
                //_log(debug, "buffers[0]=%08x", buf[0]);

                SDL_ShaderBindRawTexture(encoder, scaled.data(), PPU_WIDTH * window_scale, PPU_HEIGHT * window_scale);
                SDL_ShaderSetUniform1I(encoder, "iFrame", frames_rendered);
                SDL_ShaderSetUniform1I(decoder, "iFrame", frames_rendered);
                SDL_ShaderSetUniform1I(hermite, "iFrame", frames_rendered);
                SDL_ShaderSetUniform1I(process, "iFrame", frames_rendered);
                SDL_ShaderSetUniform1F(encoder, "iTime", time);
                SDL_ShaderSetUniform1F(decoder, "iTime", time);
                SDL_ShaderSetUniform1F(hermite, "iTime", time);
                SDL_ShaderSetUniform1F(process, "iTime", time);
                SDL_UseShaderWithFramebuffer(encoder, buffers[0].data());
                SDL_ShaderSetUniform1I(encoder, "iFrame", frames_rendered);
                SDL_ShaderSetUniform1I(decoder, "iFrame", frames_rendered);
                SDL_ShaderSetUniform1I(hermite, "iFrame", frames_rendered);
                SDL_ShaderSetUniform1I(process, "iFrame", frames_rendered);
                SDL_ShaderSetUniform1F(encoder, "iTime", time);
                SDL_ShaderSetUniform1F(decoder, "iTime", time);
                SDL_ShaderSetUniform1F(hermite, "iTime", time);
                SDL_ShaderSetUniform1F(process, "iTime", time);
                SDL_UseShaderWithFramebuffer(decoder, buffers[1].data());
                SDL_ShaderSetUniform1I(encoder, "iFrame", frames_rendered);
                SDL_ShaderSetUniform1I(decoder, "iFrame", frames_rendered);
                SDL_ShaderSetUniform1I(hermite, "iFrame", frames_rendered);
                SDL_ShaderSetUniform1I(process, "iFrame", frames_rendered);
                SDL_ShaderSetUniform1F(encoder, "iTime", time);
                SDL_ShaderSetUniform1F(decoder, "iTime", time);
                SDL_ShaderSetUniform1F(hermite, "iTime", time);
                SDL_ShaderSetUniform1F(process, "iTime", time);
                SDL_UseShaderWithFramebuffer(hermite, buffers[2].data());
                SDL_ShaderSetUniform1I(encoder, "iFrame", frames_rendered);
                SDL_ShaderSetUniform1I(decoder, "iFrame", frames_rendered);
                SDL_ShaderSetUniform1I(hermite, "iFrame", frames_rendered);
                SDL_ShaderSetUniform1I(process, "iFrame", frames_rendered);
                SDL_ShaderSetUniform1F(encoder, "iTime", time);
                SDL_ShaderSetUniform1F(decoder, "iTime", time);
                SDL_ShaderSetUniform1F(hermite, "iTime", time);
                SDL_ShaderSetUniform1F(process, "iTime", time);
                SDL_UseShader(process);
                // SDL_UseShaderWithFramebuffer(process, buffers[3].data());
                // SDL_ShaderSetUniform1I(encoder, "iFrame", frames_rendered);
                // SDL_ShaderSetUniform1I(decoder, "iFrame", frames_rendered);
                // SDL_ShaderSetUniform1I(hermite, "iFrame", frames_rendered);
                // SDL_ShaderSetUniform1I(process, "iFrame", frames_rendered);
                // SDL_ShaderSetUniform1F(encoder, "iTime", time);
                // SDL_ShaderSetUniform1F(decoder, "iTime", time);
                // SDL_ShaderSetUniform1F(hermite, "iTime", time);
                // SDL_ShaderSetUniform1F(process, "iTime", time);
    
                // SDL_UseShader(curvature);
                //SDL_UseShaderWithFramebuffer(curvature, buffers[4].data());

                time += 1.0f / 60.0f;
            } else {
                SDL_UpdateTexture(
                    sdl::texture,
                    NULL,
                    buf,
                    PPU_WIDTH * sizeof(uint32_t)
                );

                SDL_RenderCopy(sdl::renderer, sdl::texture, NULL, NULL);
            }

            if (ntsc_codec_enabled) {
                SDL_GL_SwapWindow(sdl::window);
            } else {
                SDL_RenderPresent(sdl::renderer);
            }

            frames_rendered++;

            SDL_Event event;

            while (SDL_PollEvent(&event)) {
                switch (event.type) {
                    case SDL_DROPFILE: { rom_drop_cb(event.drop.file); } break;
                    case SDL_QUIT: { close(); } break;
                    case SDL_KEYDOWN: {
                        switch (event.key.keysym.sym) {
                            case SDLK_LCTRL: { lctrl_down = true; } break;
                            case SDLK_r: {
                                if (!lctrl_down) break;

                                gameboy::reload_rom(cli::setting("cartridge", "geebly-no-cart").c_str());
                            } break;
                            case SDLK_F1: { gameboy::spu::mute_ch1 = !gameboy::spu::mute_ch1; } break;
                            case SDLK_F2: { gameboy::spu::mute_ch2 = !gameboy::spu::mute_ch2; } break;
                            case SDLK_F3: { gameboy::spu::mute_ch3 = !gameboy::spu::mute_ch3; } break;
                            case SDLK_F4: { gameboy::spu::mute_ch4 = !gameboy::spu::mute_ch4; } break;
                            case SDLK_ESCAPE: {
                                gameboy::mute();
                                ui::show();
                                gameboy::unmute();
                            } break;
                            case SDLK_1: {
                                gameboy::save_state("quick.ss");
                                _log(ok, "Saved quick save state");
                            } break;
                            case SDLK_2: {
#ifdef _WIN32
                                bool compatible = gameboy::load_state("quick.ss");

                                if (!compatible) {
                                    gameboy::mute();

                                    std::string text = "This Quick Save State was created for \""
                                        + gameboy::state_rom + "\"\n\nLoad anyways?";
                                    
                                    int result = fd::message_box(
                                        text.c_str(),
                                        "Save Incompatible",
                                        MB_YESNO | MB_ICONWARNING | MB_APPLMODAL | MB_DEFBUTTON2,
                                        get_hwnd()
                                    );

                                    gameboy::unmute();

                                    if (result == IDYES) {
                                        gameboy::load_state("quick.ss", true);
                                        _log(info, "Loaded quick save state");
                                    } else {
                                        _log(info, "Cancelled Quick Save State load");
                                        break;
                                    }
                                } else {
                                    _log(info, "Loaded quick save state");
                                }
#else
                                bool compatible = true;

                                gameboy::load_state("quick.ss", true);

                                _log(info, "Loaded quick save state");
#endif
                            } break;

#ifdef _WIN32
                            case SDLK_9: {
                                gameboy::mute();

                                std::string name = fd::save_as("Save state as", ".ss", "Geebly Save State\0*.ss\0\0", OFN_OVERWRITEPROMPT);

                                if (name.size()) {
                                    gameboy::save_state(name);

                                    name = name.substr(name.find_last_of('\\') + 1);

                                    _log(ok, "Saved state \"%s\"", name.c_str());
                                } else {
                                    _log(info, "No filename specified for save state");
                                }

                                gameboy::unmute();
                            } break;

                            case SDLK_0: {
                                gameboy::mute();
                            
                                std::string name = fd::open("Open state", ".ss", "Geebly Save State\0*.ss\0\0");

                                if (name.size()) {
                                    bool compatible = gameboy::load_state(name);

                                    if (!compatible) {
                                        std::string text = "This Save State was created for \""
                                            + gameboy::state_rom + "\"\n\nLoad anyways?";
                                        
                                        int result = fd::message_box(
                                            text.c_str(),
                                            "Save Incompatible",
                                            MB_YESNO | MB_ICONWARNING | MB_APPLMODAL | MB_DEFBUTTON2,
                                            get_hwnd()
                                        );

                                        gameboy::unmute();

                                        if (result == IDYES) {
                                            gameboy::load_state(name, true);

                                            name = name.substr(name.find_last_of('\\') + 1);

                                            _log(ok, "Loaded state \"%s\"", name.c_str());
                                        } else {
                                            _log(info, "Cancelled Save State load");
                                            break;
                                        }
                                    } else {
                                        _log(ok, "Loaded state \"%s\"", name.c_str());
                                    }
                                } else {
                                    _log(info, "No filename specified for save state");
                                }

                                gameboy::unmute();
                            } break;
#endif
                            case SDLK_3: {
                                gameboy::mute();

                                SDL_FlushEvent(SDL_KEYDOWN);
                                std::string addr_str = ui::input_hex(4);

                                if (!addr_str.size()) {
                                    gameboy::unmute();

                                    break;
                                }

                                uint16_t addr = std::stoi("0x" + addr_str, nullptr, 16);

                                std::string val_str = ui::input_hex(4);

                                gameboy::unmute();

                                if (!val_str.size()) {
                                    _log(info, "Cancelled write to $%04x", addr);

                                    break;
                                }
                                
                                uint16_t val = std::stoi("0x" + val_str, nullptr, 16);

                                gameboy::bus::write(addr, val, val_str.size() >> 1);

                                _log(debug, "Wrote 0x%02x to $%04x", val, addr);
                            } break;
                            default: { keydown_cb(event.key.keysym.sym); } break;
                        }
                    } break;
                    case SDL_KEYUP: {
                        switch (event.key.keysym.sym) {
                            case SDLK_LCTRL: { lctrl_down = false; } break;
                            default: { keyup_cb(event.key.keysym.sym); } break;
                        }
                        
                    } break;
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