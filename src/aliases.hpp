#pragma once

#include <cstdint>

namespace gameboy {
    typedef uint32_t u32;
    typedef uint16_t u16;
    typedef uint8_t  u8;
    typedef  int8_t  s8;
    typedef std::size_t size_t;

    namespace utility {
        inline u32 default_mb_read(u8* buffer, u16 addr, size_t size, size_t region_offset) {
            u32 d = 0;
            while (size) {
                d |= buffer[(addr+(size-1))-region_offset] << (((size--)-1)*8);
            }
            return d;
        }
        
        inline void default_mb_write(u8* buffer, u16 addr, u16 value, size_t size, size_t region_offset) {
            while (size--) {
                buffer[(addr+size)-region_offset] = (value & (0xff << size*8)) >> (size*8);
            }
        }
    }
    
    bool debugger_enabled = false,
         bios_checks_enabled = false,
         inaccessible_vram_emulation_enabled = true,
         window_closed = false,
         skip_bootrom = false;
}