#pragma once

#include "../common.hpp"
#include "../theme.hpp"

//#define GPU_PRIMITIVES_GL_STANDALONE
#include "../primitives.hpp"

#include "image.hpp"

#include <iostream>

namespace frontend {
    namespace ui {
        enum checkbox_state_t {
            CBS_NONE,
            CBS_HOVERED,
            CBS_PRESSED
        };

        typedef void (*checkbox_callback_t)(bool);

        class checkbox_t : public element_t {
            checkbox_state_t state;

            // Color animation
            int frames = 0, target_frames = 0;
            SDL_Color original_color = { 0, 0, 0, 0 }, target_color = { 0, 0, 0, 0 };
        
        public:
            bool checked = false;

            int alignment;

            void set_alignment(int alignment) {
                this->alignment = alignment;
            }

            image_t* unchecked_icon = new image_t,
                   * checked_icon = new image_t;
            
            image_t* icon = nullptr;

            SDL_Color background_color = { 0, 0, 0, 0 };
            double scale, scale_x, scale_y, radius = 0.0;
            checkbox_callback_t callback;
            bool square = false, align = false;

            void init(SDL_Window* window, SDL_Renderer* renderer) override {
                unchecked_icon->alignment = ALIGN_CENTER;
                unchecked_icon->path = "res/checkbox-empty.png";
                unchecked_icon->square = true;
                unchecked_icon->scale_x = 0.75;
                unchecked_icon->init(window, renderer);

                checked_icon->alignment = ALIGN_CENTER;
                checked_icon->path = "res/checkbox-full.png";
                checked_icon->square = true;
                checked_icon->scale_x = 0.75;
                checked_icon->init(window, renderer);

                icon = checked ? checked_icon : unchecked_icon;
            }

            void draw(SDL_Window* window, SDL_Renderer* renderer, box_t* bounds = nullptr) override {
                SDL_Rect rect;

                box_t element_bounds;

                rect.h = b.h * scale * scale_y;
                rect.w = rect.h;
                rect.x = b.x + ((b.w - rect.w) / 2);
                rect.y = b.y + ((b.h - rect.h) / 2);

                if (align)
                    switch (get_default_horizontal_alignment(alignment)) {
                        case ALIGNH_LEFT: { rect.x = b.x - rect.w; }
                    }

                b = *(box_t*)&rect;

                primitives::render_rounded_rectangle_filled(
                    renderer,
                    rect.x, rect.y, rect.w, rect.h,
                    background_color.r,
                    background_color.g,
                    background_color.b,
                    b.h * radius
                );
                
                icon->set_bounds(*(box_t*)&rect);
                icon->draw(window, renderer, &element_bounds);

                GEEBLY_UI_DEBUG_DRAW_BOUNDS(rect.);

                if (bounds) *bounds = *(box_t*)&rect;
            }

            void update(SDL_Window* window, SDL_Event* event, bool* redraw) override {
                switch (event->type) {
                    case SDL_MOUSEMOTION: {
                        if (is_inside_box(this->b, { event->motion.x, event->motion.y })) {
                            if (state != CBS_HOVERED) {
                                original_color = background_color;
                                target_color = theme::button_hovered_background;
                                target_frames = button_animation_length;

                                *redraw = true;
                            }

                            state = CBS_HOVERED;
                        } else {
                            if (state != CBS_NONE) {
                                original_color = background_color;
                                target_color = theme::button_default_background;
                                target_frames = button_animation_length;

                                *redraw = true;
                            }

                            state = CBS_NONE;
                        }
                    } break;
                    case SDL_MOUSEBUTTONDOWN: {
                        if (is_inside_box(this->b, { event->button.x, event->button.y })) {
                            if (state != CBS_PRESSED) {
                                original_color = background_color;
                                target_color = theme::button_pressed_background;
                                target_frames = button_animation_length;

                                *redraw = true;
                            }

                            state = CBS_PRESSED;
                        }
                    } break;
                    case SDL_MOUSEBUTTONUP: {
                        if (is_inside_box(this->b, { event->button.x, event->button.y })) {
                            if (state != CBS_HOVERED) {
                                original_color = background_color;
                                target_color = theme::button_hovered_background;
                                target_frames = button_animation_length;

                                checked = !checked;

                                icon = checked ? checked_icon : unchecked_icon;

                                callback(checked);

                                *redraw = true;
                            }

                            state = CBS_HOVERED;
                        } else {
                            if (state != CBS_NONE) {
                                original_color = background_color;
                                target_color = theme::button_default_background;
                                target_frames = button_animation_length;

                                *redraw = true;
                            }

                            state = CBS_NONE;
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
                checked_icon->unload();
                unchecked_icon->unload();
                //text.~basic_string();
                //delete &state;
                //delete &background_color;
                //delete &text_color;
                //delete &scale;
            }
        };
    }
}