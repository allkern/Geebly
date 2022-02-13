#pragma once

// Resource manager

#include "SDL.h"
#include "SDL_ttf.h"

#ifndef STBI_INCLUDE_STB_IMAGE_H
#define STB_IMAGE_IMPLEMENTATION
#include "../stb_image.h"
#endif

#include <vector>
#include <string>
#include <algorithm>

namespace frontend {
    namespace ui {
        namespace rm {
            struct font_t {
                TTF_Font* ptr;
                std::string path;
                int size;
            };

            struct image_t {
                stbi_uc* ptr;
                std::string path;
                int width, height;
            };

            std::vector <font_t> fonts;
            std::vector <image_t> images;

            image_t* load_image(std::string path) {
                // Check if the image has already been loaded
                auto i = std::find_if(std::begin(images), std::end(images),
                    [path](image_t& image){ return image.path == path; }
                );

                // If so, then return a pointer to it
                if (i != std::end(images)) {
                    return &*i;
                }

                // Else load a new image
                image_t image;

                image.ptr = stbi_load(path.c_str(), &image.width, &image.height, nullptr, STBI_rgb_alpha);
                image.path = path;

                images.push_back(image);

                return &images.at(images.size() - 1);
            }

            font_t* load_font(std::string path, int size) {
                // Check if a font with the same path and size has already been loaded
                auto f = std::find_if(std::begin(fonts), std::end(fonts),
                    [path, size](font_t& font){ return (font.path == path) && (font.size == size); }
                );

                // If so, return its ID
                if (f != std::end(fonts))
                    return &*f;

                // Else load a new font
                font_t font;

                font.ptr = TTF_OpenFont(path.c_str(), size);
                font.path = path;
                font.size = size;

                fonts.push_back(font);

                return &fonts.at(fonts.size() - 1);
            }

            void unload_all() {
                // Close all fonts
                for (font_t& f : fonts) TTF_CloseFont(f.ptr);

                // Free all images
                for (image_t& i : images) stbi_image_free(i.ptr);

                fonts.clear();
                images.clear();
            }
        }
    }
}