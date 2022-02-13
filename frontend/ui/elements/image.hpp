#pragma once

#include "../common.hpp"

#ifndef STBI_INCLUDE_STB_IMAGE_H
#define STB_IMAGE_IMPLEMENTATION
#include "../../stb_image.h"
#endif

#include "../resources.hpp"

namespace frontend {
    namespace ui {
        class image_t : public element_t {
            SDL_Texture* texture = nullptr;
            rm::image_t* image = nullptr;
        public:
            int alignment = 0;

            void set_alignment(int alignment) {
                this->alignment = alignment;
            }

            std::string path;
            double scale_x = 0.0, scale_y = 0.0;
            bool square = false;

            void init(SDL_Window* window, SDL_Renderer* renderer) override {
                image = rm::load_image(path);

                texture = SDL_CreateTexture(
                    renderer,
                    SDL_PIXELFORMAT_ABGR8888,
                    SDL_TEXTUREACCESS_STATIC,
                    image->width, image->height
                );

                SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
                SDL_SetTextureScaleMode(texture, SDL_ScaleModeBest);

                SDL_UpdateTexture(texture, NULL, image->ptr, image->width * sizeof(uint32_t));
            }

            void draw(SDL_Window* window, SDL_Renderer* renderer, box_t* bounds = nullptr) override {
                SDL_Rect rect;

                rect = *(SDL_Rect*)&b;

                if (square) b.w = b.h;
                if (square) rect.w = b.h;

                if (square) {
                    rect.w *= scale_x;
                    rect.h *= scale_x;
                } else {
                    rect.w *= scale_x;
                    rect.h *= scale_y;
                }

                switch (get_default_vertical_alignment(alignment)) {
                    case VTOP: break;
                    case VCENTER: { rect.y = b.y + (b.h / 2) - (rect.h / 2); } break;
                    case VBOTTOM: { rect.y = b.y + b.h - rect.h; } break;
                }

                switch (get_default_horizontal_alignment(alignment)) {
                    case HLEFT: break;
                    case HCENTER: { rect.x = b.x + (b.w / 2) - (rect.w / 2); } break;
                    case HRIGHT: { rect.x = b.x + b.w - rect.w; } break;
                }

                SDL_RenderCopy(renderer, texture, NULL, &rect);

                GEEBLY_UI_DEBUG_DRAW_BOUNDS(b.);

                if (bounds) *bounds = b;
            }

            void update(SDL_Window* window, SDL_Event* event, bool* redraw) override {}

            void unload() override {
                SDL_DestroyTexture(texture);
                stbi_image_free(image->ptr);
            }
        };
    }
}