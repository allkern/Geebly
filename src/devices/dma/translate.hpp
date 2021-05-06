#pragma once

#include "../cart.hpp"
#include "../wram.hpp"
#include "../ppu/memory.hpp"

namespace gameboy {
    namespace dma {
        u8* translate(u16 addr) {
            if (addr <= ROM_END) { return cart::ptr(addr); }
            if (addr >= SRAM_END && addr <= SRAM_END) { return cart::ptr(addr); }
            if (addr >= WRA0_BEGIN && addr <= WRA0_END) { return &wram::wram[0][addr-WRA0_BEGIN]; }
            if (addr >= WRAX_BEGIN && addr <= WRAX_END) { return &(wram::wram[(!wram::current_bank_idx) ? 1 : wram::current_bank_idx].at(addr-WRAX_BEGIN)); }
            if (addr >= SRAM_BEGIN && addr <= SRAM_END) { return cart::ptr(addr); }
            if (addr >= VRAM_BEGIN && addr <= VRAM_END) { return &ppu::vram[ppu::current_bank_idx].at(addr-PPU_BEGIN); }
            if (addr >= OAM_BEGIN && addr <= OAM_END) { return &ppu::oam[addr-OAM_BEGIN]; }

            return nullptr;
        }
    }
}
