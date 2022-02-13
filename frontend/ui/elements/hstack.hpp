#pragma once

#include "../common.hpp"

namespace frontend {
    namespace ui {
        struct hstack_t : public element_t {
            int alignment = 0;

            void set_alignment(int alignment) {
                this->alignment = alignment;
            }
            
            std::vector <element_t*> elements;

            int side_padding = 0, element_padding = 0;

            void push_element(element_t* element) {
                elements.push_back(element);
            }

            void draw(SDL_Window* window, SDL_Renderer* renderer, box_t* bounds = nullptr) override {
                //GEEBLY_UI_DEBUG_DRAW_BOUNDS(b.);

                // Initial x and last element width
                int x, lw = 0;

                switch (get_default_horizontal_alignment(alignment)) {
                    case ALIGNH_LEFT: { x = b.x + side_padding; } break;
                    case ALIGNH_RIGHT: { x = b.x + b.w - side_padding; } break;
                }

                // Setup a box so we get the last element width
                box_t e;

                for (element_t* element : elements) {
                    // Only set top left coords and height
                    // Leave width to the element
                    box_t bounds;
                    bounds.x = x;
                    bounds.y = b.y;
                    bounds.h = b.h;

                    element->set_bounds_masked(bounds, MASK_W);
                    element->draw(window, renderer, &e);

                    // Set last width
                    lw = e.w;

                    // Accumulate last element width
                    switch (get_default_horizontal_alignment(alignment)) {
                        case ALIGNH_LEFT: { x += lw; x += element_padding; } break;
                        case ALIGNH_RIGHT: { x -= lw; x -= element_padding; } break;
                    }
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