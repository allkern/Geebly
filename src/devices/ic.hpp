#pragma once

#include "../aliases.hpp"

#define MMIO_IF 0xff0f
#define MMIO_IE 0xffff

#define VBL_INT   0x1
#define STAT_INT  0x2
#define TIMER_INT 0x4
#define JOYP_INT  0x10

namespace gameboy {
    namespace ic {
        u8 dummy;

        u8 ia = 0, ie = 0;

        u32 read(u16 addr, size_t size) {
            if (addr == MMIO_IF) { return ia; }
            if (addr == MMIO_IE) { return ie; }
            return 0;
        }

        void write(u16 addr, u16 value, size_t size) {
            if (addr == MMIO_IF) { ia = value & 0xff; return; }
            if (addr == MMIO_IE) { ie = value & 0xff; return; }
        }

        u8& ref(u16 addr) {
            if (addr == MMIO_IF) { return ia; }
            if (addr == MMIO_IE) { return ie; }
            return dummy;
        }
    }
}