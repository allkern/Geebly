#pragma once

#include "../aliases.hpp"
#include "../global.hpp"

#include "ic.hpp"
#include "../cpu/registers.hpp"

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

        int delay = 0;
        bool irq = false;

        void keydown(int k) {
            switch (k) {
                case JOYP_START : { buttons &= ~JOYP_START;  irq = true; } break;
                case JOYP_SELECT: { buttons &= ~JOYP_SELECT; irq = true; } break;
                case JOYP_B     : { buttons &= ~JOYP_B;      irq = true; } break;
                case JOYP_A     : { buttons &= ~JOYP_A;      irq = true; } break;
                case JOYP_RIGHT : { buttons &= ~JOYP_RIGHT;  irq = true; } break;
                case JOYP_LEFT  : { buttons &= ~JOYP_LEFT;   irq = true; } break;
                case JOYP_UP    : { buttons &= ~JOYP_UP;     irq = true; } break;
                case JOYP_DOWN  : { buttons &= ~JOYP_DOWN;   irq = true; } break;
                default: break;
            }

            if (irq) stopped = false;

            if (settings::enable_joyp_irq_delay) {
                if (irq) delay = rand() % 1000;
            } else {
                ic::ref(MMIO_IF) |= IRQ_JOYP;
            }
        }

        void keyup(int k) {
            buttons |= k;
        }

        void write(u8 value) {
            button = !(value & 0b00100000);
            direct = !(value & 0b00010000);
        }

        u8 read() {
            if (button) return 0xd0 | (buttons & 0xf);
            if (direct) return 0xe0 | (buttons >> 4);

            return 0xff;
        }

        void update() {
            if (irq) {
                if (!(delay--)) {
                    ic::ref(MMIO_IF) |= IRQ_JOYP;
                    delay = 0; irq = 0;
                }
            }
        }
    }
}