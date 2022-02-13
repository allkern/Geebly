#pragma once

#include "SDL.h"

namespace frontend {
    namespace ui {
        namespace theme {
            // Solarized
            // SDL_Color default_background = { 0, 43, 54, SDL_ALPHA_OPAQUE },
            //           button_default_background = { 7, 54, 66, SDL_ALPHA_OPAQUE },
            //           button_hovered_background = { 88, 110, 117, SDL_ALPHA_OPAQUE },
            //           button_pressed_background = { 101, 123, 131, SDL_ALPHA_OPAQUE },
            //           button_text = { 224, 224, 224, SDL_ALPHA_OPAQUE },
            //           button_secondary_text = { 200, 200, 200, SDL_ALPHA_OPAQUE };

            // Dark
            SDL_Color default_background = { 31, 31, 31, SDL_ALPHA_OPAQUE },
                      button_default_background = { 41, 41, 41, SDL_ALPHA_OPAQUE },
                      button_hovered_background = { 61, 61, 61, SDL_ALPHA_OPAQUE },
                      button_pressed_background = { 91, 91, 91, SDL_ALPHA_OPAQUE },
                      button_disabled_background = { 51, 51, 51, SDL_ALPHA_OPAQUE },
                      button_text = { 224, 224, 224, SDL_ALPHA_OPAQUE },
                      button_secondary_text = { 184, 184, 184, SDL_ALPHA_OPAQUE },
                      button_disabled_text = { 124, 124, 124, SDL_ALPHA_OPAQUE };

            // Light
            // SDL_Color default_background = { 224, 224, 224, SDL_ALPHA_OPAQUE },
            //           button_default_background = { 214, 214, 214, SDL_ALPHA_OPAQUE },
            //           button_hovered_background = { 194, 194, 194, SDL_ALPHA_OPAQUE },
            //           button_pressed_background = { 164, 164, 164, SDL_ALPHA_OPAQUE },
            //           button_text = { 31, 31, 31, SDL_ALPHA_OPAQUE },
            //           button_secondary_text = { 71, 71, 71, SDL_ALPHA_OPAQUE };
        }
    }
}