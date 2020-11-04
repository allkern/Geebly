#pragma once

#include <array>

#include "../aliases.hpp"

#define WRAM_BEGIN 0xc000
#define WRAM_END 0xdfff

#define WRA0_BEGIN WRAM_BEGIN
#define WRA0_END 0xcfff
#define WRA1_BEGIN 0xd000
#define WRA1_END WRAM_END

namespace gameboy {
    namespace wram {
        typedef std::array<u8, 0x1000> wram_t;

        u8 dummy;

        wram_t bank0, bank1;

        u32 read(u16 addr, size_t size) {
            u32 d = 0;
            if (addr >= WRA0_BEGIN && addr <= WRA0_END) {
                return utility::default_mb_read(bank0.data(), addr, size, WRA0_BEGIN);
            }
                
            if (addr >= WRA1_BEGIN && addr <= WRA1_END) {
                return utility::default_mb_read(bank1.data(), addr, size, WRA0_BEGIN);
            }
            return 0;
        }

        void write(u16 addr, u16 value, size_t size) {
            if (addr >= WRA0_BEGIN && addr <= WRA0_END) {
                utility::default_mb_write(bank0.data(), addr, value, size, WRA0_BEGIN);
                return;
            }

            if (addr >= WRA1_BEGIN && addr <= WRA1_END) {
                utility::default_mb_write(bank1.data(), addr, value, size, WRA1_BEGIN);
                return;
            }
        }

        u8& ref(u16 addr) {
            if (addr >= WRA0_BEGIN && addr <= WRA0_END) { return bank0[addr-WRA0_BEGIN]; }
            if (addr >= WRA1_BEGIN && addr <= WRA1_END) { return bank1[addr-WRA1_BEGIN]; }
            return dummy;
        }
    }
}