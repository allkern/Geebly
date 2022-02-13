#pragma once

#include "../common.hpp"
#include "../theme.hpp"

//#define GPU_PRIMITIVES_GL_STANDALONE
#include "../primitives.hpp"

#include <iostream>

namespace frontend {
    namespace ui {
        enum button_state_t {
            BS_NONE,
            BS_HOVERED,
            BS_PRESSED
        };

        typedef void (*button_pressed_callback_t)(void);

        class button_t : public element_t {
            button_state_t state;

            // Color animation
            int frames = 0, target_frames = 0;
            SDL_Color original_color = { 0, 0, 0, 0 }, target_color = { 0, 0, 0, 0 };
        
        public:
            int alignment;

            void set_alignment(int alignment) {
                this->alignment = alignment;
            }

            element_t* element = nullptr;
            SDL_Color background_color = { 0, 0, 0, 0 };
            double scale, scale_x, scale_y;
            button_pressed_callback_t callback;
            bool square = false, disabled = false, align = false;
            bool update_element = true;
            double radius = 0.0;

            void set_element(element_t* element) {
                this->element = element;
            }

            void draw(SDL_Window* window, SDL_Renderer* renderer, box_t* bounds = nullptr) override {
                SDL_Rect rect;

                box_t element_bounds;

                rect.w = b.w * scale * scale_x;
                rect.h = b.h * scale * scale_y;
                rect.x = b.x + ((b.w - rect.w) / 2);
                rect.y = b.y + ((b.h - rect.h) / 2);

                if (square) rect.w = rect.h;

                if (align)
                    switch (get_default_horizontal_alignment(alignment)) {
                        case ALIGNH_LEFT: { rect.x = b.x - rect.w; }
                    }

                b = *(box_t*)&rect;

                _log(debug, "rect=%u,%u,%u,%u", rect.w, rect.h, rect.x, rect.y);

                primitives::render_rounded_rectangle_filled(
                    renderer,
                    rect.x, rect.y, rect.w, rect.h,
                    background_color.r,
                    background_color.g,
                    background_color.b,
                    radius
                );

                if (element) {
                    element->set_bounds(*(box_t*)&rect);
                    element->draw(window, renderer, &element_bounds);
                }

                GEEBLY_UI_DEBUG_DRAW_BOUNDS(rect.);

                if (bounds) *bounds = *(box_t*)&rect;
            }

            void update(SDL_Window* window, SDL_Event* event, bool* redraw) override {
                if (update_element && element)
                    element->update(window, event, redraw);

                if (disabled) {
                    background_color = theme::button_disabled_background;

                    return;
                }

                switch (event->type) {
                    case SDL_MOUSEMOTION: {
                        if (is_inside_box(this->b, { event->motion.x, event->motion.y })) {
                            if (state != BS_HOVERED) {
                                original_color = background_color;
                                target_color = theme::button_hovered_background;
                                target_frames = button_animation_length;

                                *redraw = true;
                            }

                            state = BS_HOVERED;
                        } else {
                            if (state != BS_NONE) {
                                original_color = background_color;
                                target_color = theme::button_default_background;
                                target_frames = button_animation_length;

                                *redraw = true;
                            }

                            state = BS_NONE;
                        }
                    } break;
                    case SDL_MOUSEBUTTONDOWN: {
                        if (is_inside_box(this->b, { event->button.x, event->button.y })) {
                            if (state != BS_PRESSED) {
                                original_color = background_color;
                                target_color = theme::button_pressed_background;
                                target_frames = button_animation_length;

                                *redraw = true;
                            }

                            state = BS_PRESSED;
                        }
                    } break;
                    case SDL_MOUSEBUTTONUP: {
                        if (is_inside_box(this->b, { event->button.x, event->button.y })) {
                            if (state != BS_HOVERED) {
                                original_color = background_color;
                                target_color = theme::button_hovered_background;
                                target_frames = button_animation_length;

                                callback();

                                *redraw = true;
                            }

                            state = BS_HOVERED;
                        } else {
                            if (state != BS_NONE) {
                                original_color = background_color;
                                target_color = theme::button_default_background;
                                target_frames = button_animation_length;

                                *redraw = true;
                            }

                            state = BS_NONE;
                        }
                    } break;
                }

                if (frames != target_frames) {
                    background_color = linear_interpolate(
                        original_color,
                        target_color,
                        frames++,
                        target_frames
                    );

                    *redraw = true;
                } else {
                    if (target_color.r) {
                        background_color = target_color;
                    }
                    frames = target_frames = 0;
                }
            }

            void unload() override {
                element->unload();
                //text.~basic_string();
                //delete &state;
                //delete &background_color;
                //delete &text_color;
                //delete &scale;
            }
        };
    }
}