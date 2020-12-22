#pragma once

#include "../aliases.hpp"
#include "../global.hpp"

#include "../SFML/Window.hpp"

#include <unordered_map>

// JOYP masks
#define MMIO_JOYP   0xff00
#define JOYP_BUTTON 0b00100000
#define JOYP_DIRECT 0b00010000
#define JOYP_DOWN   0b10000000
#define JOYP_UP     0b01000000
#define JOYP_LEFT   0b00100000
#define JOYP_RIGHT  0b00010000
#define JOYP_START  0b00001000
#define JOYP_SELECT 0b00000100
#define JOYP_B      0b00000010
#define JOYP_A      0b00000001

namespace gameboy {
    namespace joypad {
        bool button, direct;
        u8 buttons = 0xff;

        void keydown(sf::Keyboard::Key k) {
            switch (k) {
                case sf::Keyboard::Enter: { buttons &= ~JOYP_START;  } break;
                case sf::Keyboard::Q    : { buttons &= ~JOYP_SELECT; } break;
                case sf::Keyboard::S    : { buttons &= ~JOYP_B;      } break;
                case sf::Keyboard::A    : { buttons &= ~JOYP_A;      } break;
                case sf::Keyboard::Right: { buttons &= ~JOYP_RIGHT;  } break;
                case sf::Keyboard::Left : { buttons &= ~JOYP_LEFT;   } break;
                case sf::Keyboard::Up   : { buttons &= ~JOYP_UP;     } break;
                case sf::Keyboard::Down : { buttons &= ~JOYP_DOWN;   } break;
                default: break;
            }
        }

        void keyup(sf::Keyboard::Key k) {
            switch (k) {
                case sf::Keyboard::Enter: { buttons |= JOYP_START;  } break;
                case sf::Keyboard::Q    : { buttons |= JOYP_SELECT; } break;
                case sf::Keyboard::S    : { buttons |= JOYP_B;      } break;
                case sf::Keyboard::A    : { buttons |= JOYP_A;      } break;
                case sf::Keyboard::Right: { buttons |= JOYP_RIGHT;  } break;
                case sf::Keyboard::Left : { buttons |= JOYP_LEFT;   } break;
                case sf::Keyboard::Up   : { buttons |= JOYP_UP;     } break;
                case sf::Keyboard::Down : { buttons |= JOYP_DOWN;   } break;
                default: break;
            }
            
        }

        void write(u8 value) {
            button = !(value & 0b00100000);
            direct = !(value & 0b00010000);
        }

        u8 read() {
            u8 r = 0;

            if (button) return 0xd0 | (buttons & 0xf);
            if (direct) return 0xe0 | (buttons >> 4);

            return 0xff;
        }
    }
}