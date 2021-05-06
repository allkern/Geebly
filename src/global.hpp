#pragma once

#include "aliases.hpp"

namespace gameboy {
    namespace settings {
        bool debugger_enabled = false,
             bios_checks_enabled = false,
             inaccessible_vram_emulation_enabled = false,
             skip_bootrom = false,
             cgb_mode = false,
             cgb_dmg_mode = false,
             enable_joyp_irq_delay = false;
    }

    float master_volume = 1.0f;

    bool stopped = false,
         window_closed = false,
         written_to_sram = false,
         debug_run = false,
         tilted_cartridge = false,
         sound_disabled = true,
         stereo = true;

    namespace utility {
        /**
         *  \brief Read from the specified buffer
         *
         *  \param buffer Pointer to a memory buffer
         *  \param addr   Address to read from
         *  \param size   Amount of bytes to read
         *  \param offset Offset of this buffer in the memory map
         * 
         *  \return LE representation of requested bytes
         */
        inline u32 default_mb_read(u8* buffer, u16 addr, size_t size, size_t offset = 0) {
            u32 d = 0;
            while (size) {
                d |= buffer[(addr+(size-1))-offset] << (((size--)-1)*8);
            }
            return d;
        }
        
        /**
         *  \brief Write to the specified buffer
         *
         *  \param buffer Pointer to a memory buffer
         *  \param addr   The address to write to
         *  \param size   The amount of bytes to write (normally 1 or 2)
         *  \param offset Offset of this buffer in the memory map
         */
        inline void default_mb_write(u8* buffer, u16 addr, u16 value, size_t size, size_t offset) {
            while (size--) buffer[(addr+size)-offset] = value >> (size*8);
        }
    }
}