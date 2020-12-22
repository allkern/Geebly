#pragma once

#include "../../aliases.hpp"

#include "translate.hpp"

#include <cstring>

#define MMIO_DMA_TRANSFER 0xff46

namespace gameboy {
    namespace dma {
        u8 dummy;

        u8 dma_transfer = 0x0;

        u32 read(u16 addr, size_t size) {
            return dma_transfer;
        }

        void write(u16 addr, u16 value, size_t size) {
            u16 base = value << 8;

            if (value > 0xf1) {
                _log(warning, "Invalid DMA transfer with base 0x%04x", base);
            }

            u8* t = translate(base);
            
            if (t) std::memcpy(ppu::oam.data(), translate(base), 0xa0);
        }

        u8& ref(u16 addr) {
            return dma_transfer;
        }
    }
}