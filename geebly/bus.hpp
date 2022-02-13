#pragma once

#include "aliases.hpp"
#include "devices/bios.hpp"
#include "devices/cart.hpp"
#include "devices/wram.hpp"
#include "devices/hram.hpp"
#include "devices/ppu/ppu.hpp"
#include "devices/joypad.hpp"
#include "devices/ic.hpp"
#include "devices/dma/dma.hpp"
#include "devices/dma/hdma.hpp"
#include "devices/timer.hpp"
#include "devices/clock.hpp"
#include "devices/serial.hpp"
#include "devices/spu/spu.hpp"

// Clean this whole file up
#define MMIO_DISABLE_BOOTROM 0xff50

namespace gameboy {
    namespace bus {
        bool bootrom_enabled = true;

        u8 ro_sink = 0;

        void save_state(std::ofstream& o) {
            GEEBLY_WRITE_VARIABLE(bootrom_enabled);
        }

        void load_state(std::ifstream& i) {
            GEEBLY_LOAD_VARIABLE(bootrom_enabled);
        }

        void init() {
            bootrom_enabled = true;

            if (settings::skip_bootrom) {
                bootrom_enabled = false;
            }
        }

        u32 read(u16 addr, size_t size) {
            // Handle a DMG BIOS/CGB BIOS read
            if ((addr <= 0xff || (addr >= 0x200 && addr <= 0x8ff)) && bootrom_enabled) {
                return bios::read(addr, size);
            }

            // Handle a cartridge header/ROM read
            if (addr <= ROM_END) { return cart::read(addr, size); }

            // Handle a cartridge RAM read
            if (addr >= SRAM_BEGIN && addr <= SRAM_END) { return cart::read(addr, size); }

            // Handle a PPU (CRAM, VRAM) read
            if (addr >= PPU_BEGIN && addr <= PPU_END) { return ppu::read(addr, size); }

            // Handle a WRAM read
            if (addr >= WRAM_BEGIN && addr <= WRAM_END) { return wram::read(addr, size); }
            if (addr >= ECHO_BEGIN && addr <= ECHO_END) { return wram::read(addr-0x2000, size); }

            // Handle an OAM read
            if (addr >= OAM_BEGIN && addr <= OAM_END) { return ppu::read(addr, size); }

            if (addr == MMIO_JOYP) { return joypad::read(); }
            if ((addr == MMIO_SB) || (addr == MMIO_SC)) { return serial::read(addr, size); }

            if (addr >= TIMER_BEGIN && addr <= TIMER_END) { return timer::read(addr, size); }

            if (addr >= SPU_BEGIN && addr <= SPU_END) { return spu::read(addr, size); }

            if (addr == MMIO_DMA_TRANSFER) { return dma::read(addr, size); }
            if (addr >= HDMA_BEGIN && addr <= HDMA_END) { return hdma::read(addr, size); }
            if (addr == MMIO_KEY1) { return clock::read(addr, size); }
            if (addr == MMIO_SVBK) { return wram::read(addr, size); }

            // Handle a read to a PPU register
            if (addr >= PPU_R_BEGIN && addr <= PPU_R_END) { return ppu::read(addr, size); }

            if (addr == 0xff69) { return ppu::read(addr, size); }

            if (addr == MMIO_VBK) { return ppu::read(addr, size); }

            if (addr >= HRAM_BEGIN && addr <= HRAM_END) { return hram::read(addr, size); }

            if (addr == MMIO_IF || addr == MMIO_IE) { return ic::read(addr, size); }

            //_log(warning, "Unhandled bus read at addr=%04x", addr);

            return 0xff;
        }

        u8& ref(u16 addr) {
            // Handle a BIOS read
            // Handle a cartridge header/ROM read
            if (addr <= ROM_END) { return ro_sink; }

            // Handle a cartridge RAM read
            if (addr >= SRAM_BEGIN && addr <= SRAM_END) { return cart::ref(addr); }

            // Handle a PPU (CRAM, VRAM) read
            if (addr >= PPU_BEGIN && addr <= PPU_END) {  return ppu::ref(addr); }

            // Handle a WRAM read
            if (addr >= WRAM_BEGIN && addr <= WRAM_END) { return wram::ref(addr); }
            if (addr >= ECHO_BEGIN && addr <= ECHO_END) { return wram::ref(addr-0x2000); }

            // Handle an OAM read
            if (addr >= OAM_BEGIN && addr <= OAM_END) { return ppu::ref(addr); }

            if (addr == MMIO_DMA_TRANSFER) { return dma::ref(addr); }
            if (addr >= HDMA_BEGIN && addr <= HDMA_END) { return hdma::ref(addr); }

            if (addr >= TIMER_BEGIN && addr <= TIMER_END) { return timer::ref(addr); }

            // Handle a read to a PPU register
            if (addr >= PPU_R_BEGIN && addr <= PPU_R_END) { return ppu::ref(addr); }

            if ((addr >= 0xff68 && addr <= 0xff6b) || addr == 0xff4f) { return ppu::ref(addr); }

            if (addr == MMIO_SVBK) { return wram::ref(addr); }

            if (addr >= HRAM_BEGIN && addr <= HRAM_END) { return hram::ref(addr); }

            if (addr == MMIO_IF || addr == MMIO_IE) { return ic::ref(addr); }

            return ro_sink;
        }

        void write(u16 addr, u16 value, size_t size) {
            // Handle a cartridge header/ROM read
            if (addr <= ROM_END) { cart::write(addr, value, size); return; }

            // Handle a cartridge RAM read
            if (addr >= SRAM_BEGIN && addr <= SRAM_END) { cart::write(addr, value, size); return; }

            // Handle a PPU (CRAM, VRAM) read
            if (addr >= PPU_BEGIN && addr <= PPU_END) { ppu::write(addr, value, size); return; }

            // Handle a WRAM read
            if (addr >= WRAM_BEGIN && addr <= WRAM_END) { wram::write(addr, value, size); return; }
            if (addr >= ECHO_BEGIN && addr <= ECHO_END) { wram::write(addr-0x2000, value, size); return; }

            // Handle an OAM read
            if (addr >= OAM_BEGIN && addr <= OAM_END) { ppu::write(addr, value, size); return; }
            if (addr >= IOAM_BEGIN && addr <= IOAM_END) { return; }

            if (addr == MMIO_JOYP) { joypad::write(value); return; }

            if ((addr == MMIO_SB) || (addr == MMIO_SC)) { serial::write(addr, value, size); return; }

            if (addr >= TIMER_BEGIN && addr <= TIMER_END) { timer::write(addr, value, size); return; }

            if (addr >= SPU_BEGIN && addr <= SPU_END) { spu::write(addr, value, size); return; }

            if (addr == MMIO_DMA_TRANSFER) { dma::write(addr, value, size); return; }
            if (addr >= HDMA_BEGIN && addr <= HDMA_END) { hdma::write(addr, value, size); return; }

            // Handle a write to a PPU register
            if (addr >= PPU_R_BEGIN && addr <= PPU_R_END) { ppu::write(addr, value, size); return; }
            if ((addr >= 0xff68 && addr <= 0xff6b) || addr == 0xff4f) { ppu::write(addr, value, size); return; }
            if (addr == MMIO_KEY1) { clock::write(addr, value, size); return; }
            if (addr == MMIO_SVBK) { wram::write(addr, value, size); return; }

            if (addr == MMIO_DISABLE_BOOTROM) { bootrom_enabled = false; return; }

            if (addr == 0xff7f) return;

            if (addr >= HRAM_BEGIN && addr <= HRAM_END) { hram::write(addr, value, size); return; }

            if (addr == MMIO_IF || addr == MMIO_IE) { ic::write(addr, value, size); return; }

            if (addr == 0xff4c) {
                settings::cgb_dmg_mode = value == 0x4;
            }

            //_log(warning, "Unhandled bus write at addr=%04x, value=%02x", addr, value);
        }
    }
}