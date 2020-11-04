#pragma once

#include "aliases.hpp"
#include "devices/bios.hpp"
#include "devices/cart.hpp"
#include "devices/wram.hpp"
#include "devices/hram.hpp"
#include "devices/ppu.hpp"
#include "devices/io.hpp"

namespace gameboy {
    namespace bus {
        u8 ro_sink = 0;

        u32 read(u16 addr, size_t size) {
            // Handle a BIOS read
            // Handle a cartridge header/ROM read
            if (addr <= CART_ROM_END) { return cart::read(addr, size); }

            // Handle a cartridge RAM read
            if (addr >= CART_RAM_BEGIN && addr <= CART_RAM_END) { return cart::read(addr, size); }

            // Handle a PPU (CRAM, VRAM) read
            if (addr >= PPU_BEGIN && addr <= PPU_END) { return ppu::read(addr, size); }

            // Handle a WRAM read
            if (addr >= WRAM_BEGIN && addr <= WRAM_END) { return wram::read(addr, size); }

            // Handle an OAM read
            if (addr >= OAM_BEGIN && addr <= OAM_END) { return ppu::read(addr, size); }

            // Handle a read to a PPU register
            if (addr >= PPU_R_BEGIN && addr <= PPU_R_END) { return ppu::read(addr, size); }

            if (addr >= HRAM_BEGIN && addr <= HRAM_END) { return hram::read(addr, size); }

            if (addr == MMIO_IF && addr == MMIO_IE) { return io::read(addr, size); }

            return 0x0;
        }

        u8& ref(u16 addr) {
            // Handle a BIOS read
            // Handle a cartridge header/ROM read
            if (addr <= CART_ROM_END) { return ro_sink; }

            // Handle a cartridge RAM read
            if (addr >= CART_RAM_BEGIN && addr <= CART_RAM_END) { return cart::ref(addr); }

            // Handle a PPU (CRAM, VRAM) read
            if (addr >= PPU_BEGIN && addr <= PPU_END) { return ppu::ref(addr); }

            // Handle a WRAM read
            if (addr >= WRAM_BEGIN && addr <= WRAM_END) { return wram::ref(addr); }

            // Handle an OAM read
            if (addr >= OAM_BEGIN && addr <= OAM_END) { return ppu::ref(addr); }

            // Handle a read to a PPU register
            if (addr >= PPU_R_BEGIN && addr <= PPU_R_END) { return ppu::ref(addr); }

            if (addr >= HRAM_BEGIN && addr <= HRAM_END) { return hram::ref(addr); }

            if (addr == MMIO_IF && addr == MMIO_IE) { return io::ref(addr); }

            return ro_sink;
        }

        void write(u16 addr, u16 value, size_t size) {
            // Handle a cartridge header/ROM read
            if (addr >= CART_ROM_BEGIN && addr <= CART_ROM_END) { cart::write(addr, value, size); return; }

            // Handle a cartridge RAM read
            if (addr >= CART_RAM_BEGIN && addr <= CART_RAM_END) { cart::write(addr, value, size); return; }

            // Handle a PPU (CRAM, VRAM) read
            if (addr >= PPU_BEGIN && addr <= PPU_END) { ppu::write(addr, value, size); return; }

            // Handle a WRAM read
            if (addr >= WRAM_BEGIN && addr <= WRAM_END) {
                wram::write(addr, value, size); return;
            }

            // Handle an OAM read
            if (addr >= OAM_BEGIN && addr <= OAM_END) { ppu::write(addr, value, size); return; }

            // Handle a read to a PPU register
            if (addr >= PPU_R_BEGIN && addr <= PPU_R_END) { ppu::write(addr, value, size); return; }

            if (addr >= HRAM_BEGIN && addr <= HRAM_END) { hram::write(addr, value, size); return; }

            if (addr == MMIO_IF && addr == MMIO_IE) { io::write(addr, value, size); return; }

            return;
        }
    }
}