#pragma once

#include "../aliases.hpp"

#define MMIO_IF 0xff0f
#define MMIO_IE 0xffff

#define IRQ_VBL   0x1
#define IRQ_STAT  0x2
#define IRQ_TIMER 0x4
#define IRQ_JOYP  0x10

namespace gameboy {
    namespace ic {
        u8 dummy;

        u8 irq = 0, ie = 0;

        void fire(u8 irqs) {
            irq |= irqs;
        }

        u32 read(u16 addr, size_t size) {
            if (addr == MMIO_IF) { return irq; }
            if (addr == MMIO_IE) { return ie; }
            return 0;
        }

        void write(u16 addr, u16 value, size_t size) {
            if (addr == MMIO_IF) { irq = value & 0xff; return; }
            if (addr == MMIO_IE) { ie = value & 0xff; return; }
        }

        u8& ref(u16 addr) {
            if (addr == MMIO_IF) { return irq; }
            if (addr == MMIO_IE) { return ie; }
            return dummy;
        }
    }
}