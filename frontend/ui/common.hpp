#pragma once

#include <vector>
#include <string>

#include "SDL_ttf.h"

#include "resources.hpp"

#define GEEBLY_UI_DEBUG_DRAW_BOUNDS(object) \
    if (debug_draw_bounds) { \
        SDL_Rect temp; \
        temp.x = object x; \
        temp.y = object y; \
        temp.w = object w; \
        temp.h = object h; \
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); SDL_RenderDrawRect(renderer, &temp); \
    }

namespace frontend {
    namespace ui {
        struct box_t {
            int x, y, w, h;
        };

        bool debug_draw_bounds = false;

        int width, height;

        struct font_t {
            TTF_Font* ptr;

            std::string path;
            int width, height;
        };

        struct render_text_t {
            SDL_Surface* surf;
            SDL_Texture* tex;
            SDL_Rect rect;
        } m_text;

        std::vector <font_t> fonts;

        box_t get_text_bounds(std::string text, TTF_Font* font) {
            box_t box;

            if (!text.size()) {
                box.w = 0;
                box.h = 0;

                return box;
            }

            SDL_Surface* surf = TTF_RenderText_Solid(font, text.c_str(), { 0, 0, 0, 0 });

            return { 0, 0, surf->w, surf->h };
        }

        box_t get_sized_text_bounds(std::string text, std::string font_path, int font_size) {
            box_t box;

            if (!text.size()) {
                box.w = 0;
                box.h = 0;

                return box;
            }

            rm::font_t* new_font = rm::load_font(font_path, font_size);

            if (!new_font) {
                std::cout << SDL_GetError() << std::endl;
            }

            SDL_Surface* surf = TTF_RenderText_Solid(new_font->ptr, text.c_str(), { 0, 0, 0, 0 });

            return { 0, 0, surf->w, surf->h };
        }

        void render_text(SDL_Renderer* renderer, std::string text, int x, int y, TTF_Font* font, uint8_t r = 255, uint8_t g = 255, uint8_t b = 255, uint8_t a = 255, SDL_Rect* bound = nullptr) {
            if (!text.size()) text = " ";

            SDL_DestroyTexture(m_text.tex);
            SDL_FreeSurface(m_text.surf);

            m_text.surf = TTF_RenderText_Blended(font, text.c_str(), { r, g, b, a });
            m_text.tex = SDL_CreateTextureFromSurface(renderer, m_text.surf);

            m_text.rect = {
                x, y,
                m_text.surf->w,
                m_text.surf->h
            };

            if (bound) *bound = m_text.rect;

            SDL_RenderCopy(renderer, m_text.tex, nullptr, &m_text.rect);
        }

        void render_text_sized(SDL_Renderer* renderer, std::string text, int x, int y, std::string font_path, int font_size, uint8_t r = 255, uint8_t g = 255, uint8_t b = 255, uint8_t a = 255, SDL_Rect* bound = nullptr) {
            rm::font_t* new_font = rm::load_font(font_path, font_size);

            render_text(renderer, text, x, y, new_font->ptr, r, g, b, a, bound);
        }

        struct point_t {
            int x, y;
        };

        inline bool is_inside_box(box_t& box, point_t p) {
            return (p.x >= box.x) && (p.x <= (box.x + box.w)) &&
                   (p.y >= box.y) && (p.y <= (box.y + box.h));
        }

        enum element_bounds_mask_t : int {
            MASK_X  = 0b1110,
            MASK_Y  = 0b1101,
            MASK_W  = 0b1011,
            MASK_H  = 0b0111,
            MASK_XY = 0b1100,
            MASK_WH = 0b0011,
        };

        class element_t {
        protected:
            box_t b;

        public:
            box_t get_bounds() { return b; }
            void set_bounds(box_t bounds) { b = bounds; }
            void set_bounds_masked(box_t bounds, int mask) {
                if (mask & (~MASK_X)) b.x = bounds.x;
                if (mask & (~MASK_Y)) b.y = bounds.y;
                if (mask & (~MASK_W)) b.w = bounds.w;
                if (mask & (~MASK_H)) b.h = bounds.h;
            }
            void set_width(int width) { b.w = width; }
            void set_height(int height) { b.h = height; }
            virtual box_t try_get_bounds() { return { 0, 0, 0, 0 }; }
            virtual void draw(SDL_Window*, SDL_Renderer*, box_t*) = 0;
            virtual void update(SDL_Window*, SDL_Event*, bool*) = 0;
            virtual void unload() = 0;
            virtual void init(SDL_Window*, SDL_Renderer*) {}
        };

        enum default_alignment_t : int {
            ALIGN_DEFAULT = 0,
            ALIGNH_DEFAULT = ALIGN_DEFAULT,
            ALIGNV_DEFAULT = ALIGN_DEFAULT,
            ALIGNH_LEFT   = 0b0000,
            ALIGNH_CENTER = 0b0001,
            ALIGNH_RIGHT  = 0b0010,
            ALIGNV_TOP    = 0b0000,
            ALIGNV_CENTER = 0b0100,
            ALIGNV_BOTTOM = 0b1000,
            ALIGN_CENTER = ALIGNH_CENTER | ALIGNV_CENTER
        };

        enum default_horizontal_alignment_t { HLEFT, HCENTER, HRIGHT };
        enum default_vertical_alignment_t { VTOP, VCENTER, VBOTTOM };

        static int get_default_vertical_alignment(int alignment) {
            return (alignment >> 2) & 0x3;
        }

        static int get_default_horizontal_alignment(int alignment) {
            return alignment & 0x3;
        }

        static inline SDL_Color linear_interpolate(SDL_Color original, SDL_Color target, int frame, int target_frames) {
            SDL_Color n = { 0, 0, 0, 0 };

            n.r = (target.r - original.r) * ((double)frame / (double)target_frames) + original.r;
            n.g = (target.g - original.g) * ((double)frame / (double)target_frames) + original.g;
            n.b = (target.b - original.b) * ((double)frame / (double)target_frames) + original.b;

            return n;
        }

        int button_animation_length = 0;
    }
}