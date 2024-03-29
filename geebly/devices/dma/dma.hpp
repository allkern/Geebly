#pragma once

#include "../../aliases.hpp"

#include "translate.hpp"

#include <cstring>

#define MMIO_DMA_TRANSFER 0xff46

namespace gameboy {
    namespace bus {
        void write(u16, u16, size_t);
        u32 read(u16, size_t);
    }
}

namespace gameboy {
    namespace dma {
        u8 dummy;

        u8 dma_transfer = 0x0;

        void reset() {
            dma_transfer = 0x0;
        }

        void save_state(std::ofstream& o) {
            GEEBLY_WRITE_VARIABLE(dma_transfer);
        }

        void load_state(std::ifstream& i) {
            GEEBLY_LOAD_VARIABLE(dma_transfer);
        }

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

            // for (int i = 0; i < 0xa0; i++) {
            //     u16 dstw = 0x8000 + ((dst + i) & 0x1ff0);

            //     bus::write(dstw, bus::read(src + i, 1), 1);
            // }
        }

        u8& ref(u16 addr) {
            return dma_transfer;
        }
    }
}