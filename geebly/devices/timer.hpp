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
        
        u16 idiv = 0x4, prev = idiv;
        u8 div = 0, tima = 0x0, tma = 0x0;

        struct tac_t {
            frequency_t f;
            bool enable;
        } tac;

        u32 read(u16 addr, size_t size) {
            if (addr == MMIO_DIV ) { return div; }
            if (addr == MMIO_TIMA) { return tima & 0xff; }
            if (addr == MMIO_TMA ) { return tma & 0xff; }
            if (addr == MMIO_TAC ) { return (tac.f & 0x2) | (tac.enable << 2); }

            return 0;
        }

        void write(u16 addr, u16 value, size_t size) {
            if (addr == MMIO_DIV ) { div = 0x0; idiv = 0x0; return; }
            if (addr == MMIO_TIMA) { tima = value & 0xff; return; }
            if (addr == MMIO_TMA ) { tma = value & 0xff; return; }
            if (addr == MMIO_TAC ) { tac.f = (frequency_t)(value & 0x3); tac.enable = (value & 0x4); return; }
        }

        u8& ref(u16 addr) {
            return dummy;
        }

        u16 value = 0x0;

        void update() {
            static const u16 mask[4] = { 0x400, 0x10, 0x40, 0x100 };

            prev = idiv;

            idiv += clock::get();
            idiv &= 0x3fff;
            div = idiv >> 8;

            u16 falling_bits = prev & (~idiv);

            if ((falling_bits & (mask[tac.f] >> 1)) && tac.enable) {
                tima++;

                if (!tima) {
                    tima = tma;
                    ic::ia |= IRQ_TIMER;
                }
            }
        }
    }
}