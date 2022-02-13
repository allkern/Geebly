#pragma once

#include "../common.hpp"
#include "../theme.hpp"

#include <iostream>

namespace frontend {
    namespace ui {
        class text_container_t : public element_t {
            rm::font_t* font = nullptr;

        public:
            int alignment;

            void set_alignment(int alignment) {
                this->alignment = alignment;
            }

            SDL_Color color = { 0, 0, 0, 0 };
            double scale = 0.0;
            double size = 0.0;
            std::string font_path;
            std::string text;
            std::string* variable = nullptr;
            bool using_variable = false;

            void draw(SDL_Window* window, SDL_Renderer* renderer, box_t* bounds = nullptr) override {
                SDL_Rect rect;

                if (using_variable) text = *variable;

                box_t tb = get_sized_text_bounds(text, font_path, b.h * size);

                int x, y;

                switch (get_default_vertical_alignment(alignment)) {
                    case VTOP: { y = b.y; } break;
                    case VCENTER: { y = b.y + (b.h / 2) - (tb.h / 2); } break;
                    case VBOTTOM: { y = b.y + b.h - tb.h; } break;
                }

                switch (get_default_horizontal_alignment(alignment)) {
                    case HRIGHT: { x = b.x; } break;
                    case HLEFT: { x = b.x - tb.w; } break;
                }

                render_text_sized(renderer, text, x, y, font_path, b.h * size,
                    color.r,
                    color.g,
                    color.b,
                    color.a,
                    &rect
                );

                GEEBLY_UI_DEBUG_DRAW_BOUNDS(rect.);

                rect.x = x;
                rect.y = y;
                rect.w = tb.w;
                rect.h = tb.h;

                if (bounds) *bounds = *(box_t*)&rect;
            }

            box_t try_get_bounds() override {
                return get_sized_text_bounds(text, font_path, b.h * size);
            }

            void update(SDL_Window* window, SDL_Event* event, bool* redraw) override {}
            void unload() override {}
        };
    }
}