#pragma once

#include "../common.hpp"

namespace frontend {
    namespace ui {
        struct list_t : public element_t {
            std::vector <element_t*> items;

        public:
            int items_height_scale, item_padding;
            bool override_item_width = false;

            void push_item(element_t* item) {
                items.push_back(item);
            }

            void draw(SDL_Window* window, SDL_Renderer* renderer, box_t* bounds = nullptr) override {
                GEEBLY_UI_DEBUG_DRAW_BOUNDS(b.);

                int item_offset = b.h * (1.0 / items_height_scale);

                int x = b.x,
                    y = b.y,
                    w = b.w,
                    h = item_offset;

                for (element_t* item : items) {
                    if (override_item_width) {
                        item->set_bounds({ x, y, w, h });
                    } else {
                        item->set_bounds_masked({ x, y, w, h }, MASK_W);
                    }

                    GEEBLY_UI_DEBUG_DRAW_BOUNDS();

                    item->draw(window, renderer, nullptr);

                    y += item_offset;
                    y += item_padding;
                }
            }

            void update(SDL_Window* window, SDL_Event* event, bool* redraw) override {
                for (element_t* item : items)
                    item->update(window, event, redraw);
            }

            void unload() override {
                for (element_t* item : items) {
                    item->unload();

                    delete item;
                }

                items.clear();
                items.~vector();
            }
        };
    }
}