#pragma once

#include <cstdint>

namespace gameboy {
    typedef uint32_t u32;
    typedef uint16_t u16;
    typedef uint8_t  u8;
    typedef  int8_t  s8;

    namespace utility {
        inline u32 default_mb_read(u8* buffer, u16 addr, size_t size, size_t region_offset) {
            u32 d = 0;
            while (size) {
                d |= buffer[(addr+(size-1))-region_offset] << (((size--)-1)*8);
            }
            return d;
        }
    }
}