#pragma once

#include "../aliases.hpp"
#include "../global.hpp"

#include "ic.hpp"
#include "../cpu/registers.hpp"

#include "sgb.hpp"

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

        int counter = 0;
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

        u8 data = 0;
        bool done = false;

        void reset() {
            done = false;
            data = 0;
            counter = 0;
            button = false;
            direct = false;
            buttons = 0xff;
            delay = 0;
            irq = false;
        }

        void save_state(std::ofstream& o) {
            GEEBLY_WRITE_VARIABLE(done);
            GEEBLY_WRITE_VARIABLE(data);
            GEEBLY_WRITE_VARIABLE(counter);
            GEEBLY_WRITE_VARIABLE(button);
            GEEBLY_WRITE_VARIABLE(direct);
            GEEBLY_WRITE_VARIABLE(buttons);
            GEEBLY_WRITE_VARIABLE(delay);
            GEEBLY_WRITE_VARIABLE(irq);
        }

        void load_state(std::ifstream& i) {
            GEEBLY_LOAD_VARIABLE(done);
            GEEBLY_LOAD_VARIABLE(data);
            GEEBLY_LOAD_VARIABLE(counter);
            GEEBLY_LOAD_VARIABLE(button);
            GEEBLY_LOAD_VARIABLE(direct);
            GEEBLY_LOAD_VARIABLE(buttons);
            GEEBLY_LOAD_VARIABLE(delay);
            GEEBLY_LOAD_VARIABLE(irq);
        }

        void write(u8 value) {
            button = !(value & 0b00100000);
            direct = !(value & 0b00010000);

            if (sgb::transfer_ongoing() && settings::sgb_mode){}
                sgb::update(direct, button);

            // SGB Packet Transfer
            if (button && direct && settings::sgb_mode){}
                sgb::start_transfer();
        }

        int counterxd = 60;

        u8 read() {
            if (settings::sgb_mode) {
                u8 value = 0xfe;

                if (button) value = 0xd0 | (buttons & 0xf);
                if (direct) value = 0xe0 | (buttons >> 4);

                return value;
            } else {
                //if (!(counterxd--)) {
                //    if (button) return 0xd0 | (buttons & 0xf) & ~JOYP_START;
                //    counterxd = 60;
                //} else {
                    if (button) return 0xd0 | (buttons & 0xf);
                //}
                if (direct) return 0xe0 | (buttons >> 4);

                return 0xff;
            }
        }

        void update() {
            if (irq) {
                if (!(delay--)) {
                    ic::fire(IRQ_JOYP);
                    delay = 0; irq = 0;
                }
            }
        }
    }
}