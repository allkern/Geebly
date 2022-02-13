#pragma once

#include "../common.hpp"

namespace frontend {
    namespace ui {
        struct stack_t : public element_t {
            std::vector <element_t*> elements;

        public:
            void push_element(element_t* element) {
                elements.push_back(element);
            }

            void draw(SDL_Window* window, SDL_Renderer* renderer, box_t* bounds = nullptr) override {
                GEEBLY_UI_DEBUG_DRAW_BOUNDS(b.);

                int x = b.x,
                    y = b.y,
                    w = b.w,
                    h = b.h;

                for (element_t* element : elements) {
                    element->set_bounds({ x, y, w, h });
                    element->draw(window, renderer, nullptr);
                }
            }

            void update(SDL_Window* window, SDL_Event* event, bool* redraw) override {
                for (element_t* element : elements)
                    element->update(window, event, redraw);
            }

            void unload() override {
                for (element_t* element : elements) {
                    element->unload();

                    delete element;
                }

                elements.clear();
                elements.~vector();
            }
        };
    }
}