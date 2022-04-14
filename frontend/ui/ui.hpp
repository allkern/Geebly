#pragma once

#include "SDL.h"
#include "SDL_ttf.h"
#ifdef _WIN32
#include "../dialog.hpp"
#endif
#include "geebly/log.hpp"

#include "common.hpp"
#include "theme.hpp"

#include "elements/button.hpp"
#include "elements/stack.hpp"
#include "elements/hstack.hpp"
#include "elements/list.hpp"
#include "elements/text.hpp"
#include "elements/image.hpp"
#include "elements/rounded_rect.hpp"
#include "elements/checkbox.hpp"

#include <vector>
#include <string>

// The UI module contains:
// - UI renderer
// - Font manager (resource manager in the future)

namespace frontend {
    namespace ui {
        SDL_Window* m_window = nullptr;
        SDL_Renderer* m_renderer = nullptr;
        SDL_Texture* m_texture = nullptr;
        bool* m_open = nullptr;
        uint32_t* m_buf = nullptr;
        SDL_Color background = { 0, 0, 0, 0 };

        element_t* root = nullptr;

        int push_font(std::string name, int size) {
            font_t font;

            std::ifstream f;

            f.open(name);

            if (!f.good()) {
                _log(warning, "Couldn't load font \"%s\"", name.c_str());

                return 0;
            }

            font.path = name;
            font.ptr = TTF_OpenFont(name.c_str(), size);

            int minx = 0, maxx = 0;

            TTF_GlyphMetrics(font.ptr, 'a', &minx, &maxx, nullptr, nullptr, nullptr);

            font.width = maxx + minx;

            fonts.push_back(font);

            return fonts.size() - 1;
        }

        font_t* get_font(int index) {
            if (index > fonts.size()) index = 0;

            return &fonts.at(index);
        }

        void init(SDL_Window* window, SDL_Renderer* renderer, SDL_Texture* tex, bool& open) {
            m_window = window;
            m_renderer = renderer;
            m_texture = tex;
            m_open = &open;
            background = { 0, 0, 0, 255 };

            SDL_GetWindowSize(window, &width, &height);

            //SDL_GL_CreateContext(window);

            primitives::init(window);

            TTF_Init();
        }

        void update(uint32_t* buf) {
            m_buf = buf;
        }

        void close() {
            *m_open = false;

            rm::unload_all();
            
            SDL_DestroyTexture(m_texture);
            SDL_DestroyRenderer(m_renderer);
            SDL_DestroyWindow(m_window);

            SDL_Quit();
        }

#ifdef _WIN32
        HWND get_hwnd() {
            SDL_SysWMinfo wmi;
            SDL_VERSION(&wmi.version);
            SDL_GetWindowWMInfo(m_window, &wmi);

            return wmi.info.win.window;
        }
#endif

        void unload_current();
        void load_settings_menu();

        std::string rom_name, rom_path, prev_rom_path;
        std::string state_name, state_path;

        bool open = true, redraw = true;

        void hide() {
            SDL_FlushEvent(SDL_TEXTINPUT);
            SDL_FlushEvent(SDL_KEYDOWN);

            open = false;
        }

        void load_main_menu() {
            int pad = width / 15;
            list_t* list = new list_t;
            list->item_padding = height / 35;
            list->set_bounds({ pad, pad, width - pad * 2, height - pad * 2 });
            list->items_height_scale = 7;

            text_container_t* open_rom_button_text = new text_container_t;
            open_rom_button_text->alignment = ALIGNH_RIGHT | ALIGNV_CENTER;
            open_rom_button_text->color = theme::button_text;
            open_rom_button_text->font_path = "ubuntu-mono.ttf";
            open_rom_button_text->scale = 1.0;
            open_rom_button_text->size = 0.45;
            open_rom_button_text->text = "Open";

            text_container_t* open_rom_button_rom_name_text = new text_container_t;
            open_rom_button_rom_name_text->alignment = ALIGNH_LEFT | ALIGNV_CENTER;
            open_rom_button_rom_name_text->color = theme::button_secondary_text;
            open_rom_button_rom_name_text->font_path = "ubuntu-mono.ttf";
            open_rom_button_rom_name_text->scale = 1.0;
            open_rom_button_rom_name_text->size = 0.37;
            open_rom_button_rom_name_text->using_variable = true;
            open_rom_button_rom_name_text->variable = &rom_name;

            image_t* open_rom_button_icon = new image_t;
            open_rom_button_icon->alignment = ALIGN_CENTER;
            open_rom_button_icon->path = "res/folder.png";
            open_rom_button_icon->square = true;
            open_rom_button_icon->scale_x = 0.45;
            open_rom_button_icon->init(m_window, m_renderer);

            hstack_t* open_rom_button_hstack = new hstack_t;
            open_rom_button_hstack->alignment = ALIGNH_LEFT;
            open_rom_button_hstack->push_element(open_rom_button_icon);
            open_rom_button_hstack->push_element(open_rom_button_text);
            open_rom_button_hstack->side_padding = 10;

            hstack_t* open_rom_button_right_hstack = new hstack_t;
            open_rom_button_right_hstack->alignment = ALIGNH_RIGHT;
            open_rom_button_right_hstack->push_element(open_rom_button_rom_name_text);
            open_rom_button_right_hstack->side_padding = 20;

            stack_t* open_rom_button_stack = new stack_t;
            open_rom_button_stack->push_element(open_rom_button_hstack);
            open_rom_button_stack->push_element(open_rom_button_right_hstack);

            button_t* open_rom_button = new button_t;
            open_rom_button->set_element(open_rom_button_stack);
            open_rom_button->set_width(width / 1.5);
            open_rom_button->background_color = theme::button_default_background;
            open_rom_button->scale = 1.0;
            open_rom_button->scale_x = 1.0;
            open_rom_button->scale_y = 1.0;
            open_rom_button->callback = [](){
#ifdef _WIN32
                std::string name = fd::open("Open ROM", ".gb", "Game Boy\0*.gb\0Game Boy Color\0*.gbc\0\0");

                if (!name.size()) return;

                prev_rom_path = rom_path;
                rom_path = name;

                size_t slash_pos = name.find_last_of('/');

                if (slash_pos == std::string::npos) {
                    slash_pos = name.find_last_of('\\');
                }

                if (slash_pos == std::string::npos) {
                    rom_name = rom_path;
                } else {
                    rom_name = name.substr(slash_pos + 1);
                }
#endif
            };

            image_t* insert_cart_button_icon = new image_t;
            insert_cart_button_icon->alignment = ALIGN_CENTER;
            insert_cart_button_icon->path = "res/arrow-right.png";
            insert_cart_button_icon->square = true;
            insert_cart_button_icon->scale_x = 0.40;
            insert_cart_button_icon->init(m_window, m_renderer);

            button_t* insert_cart_button = new button_t;
            insert_cart_button->set_element(insert_cart_button_icon);
            insert_cart_button->square = true;
            insert_cart_button->background_color = theme::button_default_background;
            insert_cart_button->scale = 1.0;
            insert_cart_button->scale_x = 1.0;
            insert_cart_button->scale_y = 1.0;
            insert_cart_button->callback = [](){
                if (rom_path.size()) {
                    if (rom_path != prev_rom_path) {
                        prev_rom_path = rom_path;
                        gameboy::reload_rom(rom_path.c_str());
                    }
                }

                if (state_path.size()) {
                    if (!gameboy::load_state(state_path)) {
#ifdef _WIN32
                        std::string text = "This Save State was created for \""
                                    + gameboy::state_rom + "\"\n\nLoad anyways?";

                        int result = fd::message_box(
                            text.c_str(),
                            "Save Incompatible",
                            MB_YESNO | MB_ICONWARNING | MB_APPLMODAL | MB_DEFBUTTON2,
                            get_hwnd()
                        );

                        if (result == IDYES) {
                            gameboy::load_state(state_path, true);
                        }
#endif
                    }

                    state_path.clear();
                }

                hide();
            };

            hstack_t* open_rom_and_insert_hstack = new hstack_t;
            open_rom_and_insert_hstack->element_padding = 8;
            open_rom_and_insert_hstack->push_element(open_rom_button);
            open_rom_and_insert_hstack->push_element(insert_cart_button);

            text_container_t* load_state_button_text = new text_container_t;
            load_state_button_text->alignment = ALIGNH_RIGHT | ALIGNV_CENTER;
            load_state_button_text->color = theme::button_text;
            load_state_button_text->font_path = "ubuntu-mono.ttf";
            load_state_button_text->scale = 1.0;
            load_state_button_text->size = 0.45;
            load_state_button_text->text = "Load state";

            text_container_t* load_state_button_state_name_text = new text_container_t;
            load_state_button_state_name_text->alignment = ALIGNH_LEFT | ALIGNV_CENTER;
            load_state_button_state_name_text->color = theme::button_secondary_text;
            load_state_button_state_name_text->font_path = "ubuntu-mono.ttf";
            load_state_button_state_name_text->scale = 1.0;
            load_state_button_state_name_text->size = 0.37;
            load_state_button_state_name_text->using_variable = true;
            load_state_button_state_name_text->variable = &state_name;

            image_t* load_state_button_icon = new image_t;
            load_state_button_icon->alignment = ALIGN_CENTER;
            load_state_button_icon->path = "res/bookmark.png";
            load_state_button_icon->square = true;
            load_state_button_icon->scale_x = 0.45;
            load_state_button_icon->init(m_window, m_renderer);

            hstack_t* load_state_button_hstack = new hstack_t;
            load_state_button_hstack->alignment = ALIGNH_LEFT;
            load_state_button_hstack->push_element(load_state_button_icon);
            load_state_button_hstack->push_element(load_state_button_text);
            load_state_button_hstack->side_padding = 10;

            hstack_t* load_state_button_right_hstack = new hstack_t;
            load_state_button_right_hstack->alignment = ALIGNH_RIGHT;
            load_state_button_right_hstack->element_padding = 8;
            load_state_button_right_hstack->push_element(load_state_button_state_name_text);
            load_state_button_right_hstack->side_padding = 20;

            stack_t* load_state_button_stack = new stack_t;
            load_state_button_stack->push_element(load_state_button_hstack);
            load_state_button_stack->push_element(load_state_button_right_hstack);

            button_t* load_state_button = new button_t;
            load_state_button->set_width(width / 1.25);
            load_state_button->set_element(load_state_button_stack);
            load_state_button->background_color = theme::button_default_background;
            load_state_button->scale = 1.0;
            load_state_button->scale_x = 1.0;
            load_state_button->scale_y = 1.0;
            load_state_button->callback = [](){
#ifdef _WIN32
                std::string name = fd::open("Open state", ".ss", "Geebly Save State\0*.ss\0\0");
                
                if (!name.size()) return;

                state_path = name;

                size_t slash_pos = name.find_last_of('/');

                if (slash_pos == std::string::npos) {
                    slash_pos = name.find_last_of('\\');
                }

                if (slash_pos == std::string::npos) {
                    state_name = rom_path;
                } else {
                    state_name = name.substr(slash_pos + 1);
                }
#endif
            };

            text_container_t* settings_button_text = new text_container_t;
            settings_button_text->alignment = ALIGNH_RIGHT | ALIGNV_CENTER;
            settings_button_text->color = theme::button_text;
            settings_button_text->font_path = "ubuntu-mono.ttf";
            settings_button_text->scale = 1.0;
            settings_button_text->size = 0.45;
            settings_button_text->text = "Settings";

            image_t* settings_button_icon = new image_t;
            settings_button_icon->alignment = ALIGN_CENTER;
            settings_button_icon->path = "res/cog.png";
            settings_button_icon->square = true;
            settings_button_icon->scale_x = 0.45;
            settings_button_icon->init(m_window, m_renderer);

            hstack_t* settings_button_hstack = new hstack_t;
            settings_button_hstack->alignment = ALIGNH_LEFT;
            settings_button_hstack->push_element(settings_button_icon);
            settings_button_hstack->push_element(settings_button_text);
            settings_button_hstack->side_padding = 10;

            button_t* settings_button = new button_t;
            settings_button->set_width(width / 2.0);
            settings_button->set_element(settings_button_hstack);
            settings_button->background_color = theme::button_default_background;
            settings_button->scale = 1.0;
            settings_button->scale_x = 1.0;
            settings_button->scale_y = 1.0;
            settings_button->callback = [](){
                //unload_current();
                load_settings_menu();
            };

            list->push_item(open_rom_and_insert_hstack);
            list->push_item(load_state_button);
            list->push_item(settings_button);

            button_animation_length = 10;

            root = list;
        }

        const static std::string model_names[] = {
            "Game Boy (DMG)",
            "Game Boy Color (GBC)",
            "Super Game Boy (SGB)"
        };

        std::string model = "";
        int model_index = 0;

        void set_mode_and_reset() {
            if (model_index == 0) { settings::cgb_mode = false; settings::sgb_mode = false; }
            if (model_index == 1) { settings::cgb_mode = true ; settings::sgb_mode = false; }
            if (model_index == 2) { settings::cgb_mode = false; settings::sgb_mode = true; }

            gameboy::reset();
        }

        void load_settings_menu() {
            model = model_names[model_index];

            text_container_t* model_select_button_text = new text_container_t;
            model_select_button_text->alignment = ALIGNH_RIGHT | ALIGNV_CENTER;
            model_select_button_text->color = theme::button_text;
            model_select_button_text->font_path = "ubuntu-mono.ttf";
            model_select_button_text->scale = 1.0;
            model_select_button_text->size = 0.45;
            model_select_button_text->text = "Model";

            text_container_t* model_select_button_model_text = new text_container_t;
            model_select_button_model_text->alignment = ALIGNH_LEFT | ALIGNV_CENTER;
            model_select_button_model_text->color = theme::button_text;
            model_select_button_model_text->font_path = "ubuntu-mono.ttf";
            model_select_button_model_text->scale = 1.0;
            model_select_button_model_text->size = 0.37;
            model_select_button_model_text->using_variable = true;
            model_select_button_model_text->variable = &model;

            image_t* model_select_right_button_icon = new image_t;
            model_select_right_button_icon->alignment = ALIGN_CENTER;
            model_select_right_button_icon->path = "res/chevron-right.png";
            model_select_right_button_icon->square = true;
            model_select_right_button_icon->scale_x = 0.75;
            model_select_right_button_icon->init(m_window, m_renderer);

            button_t* model_select_right_button = new button_t;
            model_select_right_button->align = true;
            model_select_right_button->set_alignment(ALIGNH_LEFT);
            model_select_right_button->set_element(model_select_right_button_icon);
            model_select_right_button->background_color = theme::button_default_background;
            model_select_right_button->scale = 0.5;
            model_select_right_button->scale_x = 1.0;
            model_select_right_button->scale_y = 1.0;
            model_select_right_button->square = true;
            model_select_right_button->callback = [](){
                model_index = (model_index + 1) % 3;

                model = model_names[model_index];

                set_mode_and_reset();
            };

            image_t* model_select_left_button_icon = new image_t;
            model_select_left_button_icon->alignment = ALIGN_CENTER;
            model_select_left_button_icon->path = "res/chevron-left.png";
            model_select_left_button_icon->square = true;
            model_select_left_button_icon->scale_x = 0.75;
            model_select_left_button_icon->init(m_window, m_renderer);

            button_t* model_select_left_button = new button_t;
            model_select_left_button->align = true;
            model_select_left_button->set_alignment(ALIGNH_LEFT);
            model_select_left_button->set_element(model_select_left_button_icon);
            model_select_left_button->background_color = theme::button_default_background;
            model_select_left_button->scale = 0.5;
            model_select_left_button->scale_x = 1.0;
            model_select_left_button->scale_y = 1.0;
            model_select_left_button->square = true;
            model_select_left_button->callback = [](){
                if (!model_index) model_index = 3;

                model_index = (model_index - 1) % 3;

                model = model_names[model_index];

                set_mode_and_reset();
            };

            hstack_t* model_select_button_hstack = new hstack_t;
            model_select_button_hstack->alignment = ALIGNH_LEFT;
            model_select_button_hstack->push_element(model_select_button_text);
            model_select_button_hstack->side_padding = 20;

            hstack_t* model_select_button_right_hstack = new hstack_t;
            model_select_button_right_hstack->alignment = ALIGNH_RIGHT;
            model_select_button_right_hstack->element_padding = 8;
            model_select_button_right_hstack->push_element(model_select_right_button);
            model_select_button_right_hstack->push_element(model_select_button_model_text);
            model_select_button_right_hstack->push_element(model_select_left_button);
            model_select_button_right_hstack->side_padding = 15;

            stack_t* model_select_button_stack = new stack_t;
            model_select_button_stack->push_element(model_select_button_hstack);
            model_select_button_stack->push_element(model_select_button_right_hstack);

            rounded_rect_t* model_select_rect = new rounded_rect_t;
            model_select_rect->set_element(model_select_button_stack);
            model_select_rect->background_color = theme::button_default_background;
            model_select_rect->scale = 1.0;
            model_select_rect->scale_x = 1.0;
            model_select_rect->scale_y = 1.0;

            text_container_t* skip_bootrom_text = new text_container_t;
            skip_bootrom_text->alignment = ALIGNH_RIGHT | ALIGNV_CENTER;
            skip_bootrom_text->color = theme::button_text;
            skip_bootrom_text->font_path = "ubuntu-mono.ttf";
            skip_bootrom_text->scale = 1.0;
            skip_bootrom_text->size = 0.45;
            skip_bootrom_text->text = "Skip Boot ROM";

            checkbox_t* skip_bootrom_checkbox = new checkbox_t;
            skip_bootrom_checkbox->align = true;
            skip_bootrom_checkbox->set_alignment(ALIGNH_LEFT);
            skip_bootrom_checkbox->background_color = theme::button_default_background;
            skip_bootrom_checkbox->scale = 0.75;
            skip_bootrom_checkbox->scale_x = 1.0;
            skip_bootrom_checkbox->scale_y = 1.0;
            skip_bootrom_checkbox->checked = true;
            skip_bootrom_checkbox->callback = [](bool checked){
                settings::skip_bootrom = checked;
            };
            skip_bootrom_checkbox->init(m_window, m_renderer);

            hstack_t* skip_bootrom_button_hstack = new hstack_t;
            skip_bootrom_button_hstack->alignment = ALIGNH_LEFT;
            skip_bootrom_button_hstack->push_element(skip_bootrom_text);
            skip_bootrom_button_hstack->side_padding = 20;

            hstack_t* skip_bootrom_button_right_hstack = new hstack_t;
            skip_bootrom_button_right_hstack->alignment = ALIGNH_RIGHT;
            skip_bootrom_button_right_hstack->element_padding = 8;
            skip_bootrom_button_right_hstack->push_element(skip_bootrom_checkbox);
            skip_bootrom_button_right_hstack->side_padding = 15;

            stack_t* skip_bootrom_button_stack = new stack_t;
            skip_bootrom_button_stack->push_element(skip_bootrom_button_hstack);
            skip_bootrom_button_stack->push_element(skip_bootrom_button_right_hstack);

            rounded_rect_t* skip_bootrom_rect = new rounded_rect_t;
            skip_bootrom_rect->set_element(skip_bootrom_button_stack);
            skip_bootrom_rect->background_color = theme::button_default_background;
            skip_bootrom_rect->scale = 1.0;
            skip_bootrom_rect->scale_x = 1.0;
            skip_bootrom_rect->scale_y = 1.0;

            int pad = width / 15;
            list_t* list = new list_t;
            list->item_padding = height / 35;
            list->set_bounds({ pad, pad, width - pad * 2, height - pad * 2 });
            list->override_item_width = true;
            list->items_height_scale = 7;
            list->push_item(model_select_rect);
            list->push_item(skip_bootrom_rect);

            button_animation_length = 10;

            root = list;
        }

        void unload_current() {
            if (root) {
                root->unload();
                delete root;
                root = nullptr;
            }
        }

        void show() {
            SDL_FlushEvent(SDL_TEXTINPUT);
            SDL_FlushEvent(SDL_KEYDOWN);

            SDL_Event event;

            open = true; redraw = true;

            if (root) root->update(m_window, &event, &redraw);

            while (open) {
                while (SDL_PollEvent(&event)) {
                    switch (event.type) {
                        case SDL_QUIT: { open = false; close(); } break;
                        case SDL_KEYDOWN: {
                            switch (event.key.keysym.sym) {
                                case SDLK_ESCAPE: { hide(); } break;
                                case SDLK_F3: {
                                    redraw = true;
                                    debug_draw_bounds = !debug_draw_bounds;
                                } break;
                            }
                        } break;
                    }
                }

                if (root) root->update(m_window, &event, &redraw);

                if (redraw) {
                    SDL_RenderClear(m_renderer);

                    if (theme::default_background.a != 255) {
                        SDL_UpdateTexture(
                            m_texture,
                            NULL,
                            m_buf,
                            160 * sizeof(uint32_t)
                        );

                        SDL_RenderCopy(m_renderer, m_texture, NULL, NULL);
                    }

                    if (theme::default_background.a != 0) {
                        SDL_SetRenderDrawColor(m_renderer,
                            theme::default_background.r,
                            theme::default_background.g,
                            theme::default_background.b,
                            theme::default_background.a
                        );

                        SDL_RenderFillRect(m_renderer, NULL);
                    }

                    if (root) root->draw(m_window, m_renderer, nullptr);

                    redraw = false;
                }

                SDL_RenderPresent(m_renderer);
            }
        }

        std::string input_hex(int size) {
            return "0000";
        }
    }
}