#pragma once

#include "aliases.hpp"

namespace gameboy {
    namespace settings {
        bool debugger_enabled = false,
             bios_checks_enabled = false,
             inaccessible_vram_emulation_enabled = false,
             skip_bootrom = false,
             cgb_mode = false;
    }

    bool stopped = false, window_closed = false;

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
                //buffer[(addr+size)-region_offset] = (value & (0xff << size*8)) >> (size*8);
                buffer[(addr+size)-region_offset] = value >> (size*8);
            }
        }
    }
}