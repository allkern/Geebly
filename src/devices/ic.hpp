#pragma once

#include "../aliases.hpp"

#define MMIO_IF 0xff0f
#define MMIO_IE 0xffff

namespace gameboy {
    namespace ic {
        u8 dummy;

        u8 ia = 0; // if
        u8 ie = 0;

        u32 read(u16 addr, size_t size) {
            if (addr == MMIO_IF) { return ia; }
            if (addr == MMIO_IE) { return ie; }
            return 0;
        }

        void write(u16 addr, u16 value, size_t size) {
            if (addr == MMIO_IF) { ia = value; return; }
            if (addr == MMIO_IE) { ie = value; return; }
        }

        u8& ref(u16 addr) {
            if (addr == MMIO_IF) { return ia; }
            if (addr == MMIO_IE) { return ie; }
            return dummy;
        }
    }
}