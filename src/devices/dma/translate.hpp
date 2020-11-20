#pragma once

#include "../cart.hpp"
#include "../wram.hpp"
#include "../ppu/memory.hpp"

namespace gameboy {
    namespace dma {
        u8* translate(u16 addr) {
            if (addr <= CART_ROM_END) { return cart::ptr(addr); }
            if (addr >= CART_RAM_END && addr <= CART_RAM_END) { return cart::ptr(addr); }
            if (addr >= WRAM_BEGIN && addr <= WRAM_END) { return &wram::ref(addr); }
            if (addr >= VRAM_BEGIN && addr <= VRAM_END) { return &ppu::vram[addr-VRAM_BEGIN]; }
            if (addr >= OAM_BEGIN && addr <= OAM_END) { return &ppu::oam[addr-OAM_BEGIN]; }

            return nullptr;
        }
    }
}
