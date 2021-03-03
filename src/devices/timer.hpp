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
        enum frequency_t {
            f4khz,
            f262khz,
            f65khz,
            f16khz
        };

        u8 dummy;
        u16 div = 0x4, tima = 0x0, tma = 0x0;

        struct tac_t {
            frequency_t f : 2;
            bool enable : 1;
        } tac;

        u32 read(u16 addr, size_t size) {
            if (addr == MMIO_DIV ) { return (div >> 7) & 0xff; }
            if (addr == MMIO_TIMA) { return tima & 0xff; }
            if (addr == MMIO_TMA ) { return tma & 0xff; }
            if (addr == MMIO_TAC ) { return (tac.f & 0x2) | (tac.enable << 2); }

            return 0;
        }

        void write(u16 addr, u16 value, size_t size) {
            if (addr == MMIO_DIV ) { div = 0x0; return; }
            if (addr == MMIO_TIMA) { tima = value & 0xff; return; }
            if (addr == MMIO_TMA ) { tma = value & 0xff; return; }
            if (addr == MMIO_TAC ) { tac.f = (frequency_t)(value & 0x2); tac.enable = (value & 0x4); return; }
        }

        u8& ref(u16 addr) {
            return dummy;
        }

        void update() {
            static const u8 mask[4] = { 0x40, 0x1, 0x4, 0x10 };

            div += clock::get() >> 1;

            if (tima & 0xff00) { 
                tima = tma;
                //ic::ia |= IRQ_TIMER;
            }

            if (((div >> 3) & mask[tac.f]) && tac.enable) tima++;
        }
    }
}