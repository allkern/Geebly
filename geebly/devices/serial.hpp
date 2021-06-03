#pragma once

#include "../aliases.hpp"

#define MMIO_SB 0xff01
#define MMIO_SC 0xff02

namespace gameboy {
    namespace serial {
        u8 dummy;

        u8 r[2] = { 0x0, 0x7e };

        u32 read(u16 addr, size_t size) {
            if (addr == MMIO_SB) return r[0];
            if (addr == MMIO_SC) return 0xfe;
            return 0;
        }

        void write(u16 addr, u16 value, size_t size) {
            if (addr == MMIO_SB) { r[0] = value & 0xff; return; }
            if (addr == MMIO_SC) { r[1] = value & 0xff; return; }
        }
    }
}