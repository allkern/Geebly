#pragma once

#include "../aliases.hpp"
#include "../cpu/registers.hpp"

#include "clock.hpp"
#include "ic.hpp"

#define MMIO_DIV  0xff04
#define MMIO_TIMA 0xff05
#define MMIO_TMA  0xff06
#define MMIO_TAC  0xff07
#define TIMER_BEGIN 0xff04
#define TIMER_END 0xff07

namespace gameboy {
    namespace timer {
        enum frequency_t : int {
            f4khz,
            f262khz,
            f65khz,
            f16khz
        };

        u8 dummy;
        
        u32 idiv = 0x4, prev = idiv;
        u8 div = 0, tima = 0x0, tma = 0x0;

        struct tac_t {
            frequency_t f;
            bool enable;
        } tac;

        u32 read(u16 addr, size_t size) {
            if (addr == MMIO_DIV ) { return div; }
            if (addr == MMIO_TIMA) { return tima & 0xff; }
            if (addr == MMIO_TMA ) { return tma & 0xff; }
            if (addr == MMIO_TAC ) { return 0xf8 | ((tac.f & 0x3) | (tac.enable << 2)); }

            return 0;
        }

        bool div_reset = false;

        void write(u16 addr, u16 value, size_t size) {
            if (addr == MMIO_DIV ) { div = 0x0; prev = idiv; idiv = 0x2; div_reset = true; return; }
            if (addr == MMIO_TIMA) { tima = value & 0xff; return; }
            if (addr == MMIO_TMA ) { tma = value & 0xff; return; }
            if (addr == MMIO_TAC ) { tac.f = (frequency_t)(value & 0x3); tac.enable = (value & 0x4); return; }
        }

        u8& ref(u16 addr) {
            return dummy;
        }

        u16 value = 0x0;

        void reset() {
            value = 0;
            div = 0;
            tima = 0;
            tma = 0;
            idiv = 4;
            prev = idiv;
        }

        void save_state(std::ofstream& o) {
            GEEBLY_WRITE_VARIABLE(value);
            GEEBLY_WRITE_VARIABLE(div);
            GEEBLY_WRITE_VARIABLE(tima);
            GEEBLY_WRITE_VARIABLE(tma);
            GEEBLY_WRITE_VARIABLE(idiv);
            GEEBLY_WRITE_VARIABLE(prev);
        }

        void load_state(std::ifstream& i) {
            GEEBLY_LOAD_VARIABLE(value);
            GEEBLY_LOAD_VARIABLE(div);
            GEEBLY_LOAD_VARIABLE(tima);
            GEEBLY_LOAD_VARIABLE(tma);
            GEEBLY_LOAD_VARIABLE(idiv);
            GEEBLY_LOAD_VARIABLE(prev);
        }

        void update() {
            static const u16 mask[4] = { 0x400, 0x10, 0x40, 0x100 };

            if (div_reset) {
                // prev = idiv;
                div_reset = false;
            } else {
                prev = idiv;
                idiv += clock::get();
            }

            div = (idiv >> 7) & 0xff;

            bool selected_bit_current = (idiv & (mask[tac.f] >> 1)) && tac.enable,
                 selected_bit_prev = (prev & (mask[tac.f] >> 1)) && tac.enable;
            u32 falling_bits = prev & (~idiv);

            //_log(debug, "idiv=%08x, prev=%08x, div=%02x, falling=%08x, clock=%u", idiv, prev, div, falling_bits, clock::get());

            if (!selected_bit_current && selected_bit_prev) {
                tima++;

                if (!tima) {
                    //_log(debug, "tima overflow irq fired", tima);
                    tima = tma;
                    ic::fire(IRQ_TIMER);
                } else {
                    //_log(debug, "tima incremented %02x", tima);
                }
            }
        }
    }
}