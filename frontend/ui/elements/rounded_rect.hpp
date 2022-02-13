#pragma once

#include "../common.hpp"
#include "../theme.hpp"

//#define GPU_PRIMITIVES_GL_STANDALONE
#include "../primitives.hpp"

#include <iostream>

namespace frontend {
    namespace ui {
        class rounded_rect_t : public element_t {        
        public:
            element_t* element = nullptr;
            SDL_Color background_color = { 0, 0, 0, 0 };
            double scale, scale_x, scale_y, radius = 0.0;
            bool square = false;

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

                if (square) {
                    rect.w = rect.h;
                    b = *(box_t*)&rect;
                }

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
                if (element)
                    element->update(window, event, redraw);
            }

            void unload() override {
                element->unload();
            }
        };
    }
}