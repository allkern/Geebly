#pragma once

#include "SDL_keycode.h"

#include "geebly/gameboy.hpp"

#include <algorithm>
#include <array>

using namespace gameboy;

namespace frontend {
    namespace input {
        struct keymap_t {
            SDL_Keycode k;
            int v;
        };

        std::array <keymap_t, 8> keymap = {
            keymap_t { SDLK_a     , JOYP_A },
            keymap_t { SDLK_s     , JOYP_B },
            keymap_t { SDLK_RETURN, JOYP_START },
            keymap_t { SDLK_q     , JOYP_SELECT },
            keymap_t { SDLK_UP    , JOYP_UP },
            keymap_t { SDLK_DOWN  , JOYP_DOWN },
            keymap_t { SDLK_LEFT  , JOYP_LEFT },
            keymap_t { SDLK_RIGHT , JOYP_RIGHT }
        };

        void keydown_cb(SDL_Keycode k) {
            joypad::keydown(
                std::find_if(
                    std::begin(keymap),
                    std::end(keymap),
                    [k](keymap_t km) { return km.k == k; }
                )->v
            );
        }

        void keyup_cb(SDL_Keycode k) {
            joypad::keyup(
                std::find_if(
                    std::begin(keymap),
                    std::end(keymap),
                    [k](keymap_t km) { return km.k == k; }
                )->v
            );
        }

        void remap_key(int gbk, SDL_Keycode nk) {
            std::find_if(
                std::begin(keymap),
                std::end(keymap),
                [gbk](keymap_t km) { return km.v == gbk; }
            )->k = nk;
        }
    }
}