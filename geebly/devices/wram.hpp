#pragma once

#include <array>
#include <ctime>

#include "../aliases.hpp"
#include "../global.hpp"

#define WRAM_BEGIN 0xc000
#define WRAM_END 0xdfff
#define ECHO_BEGIN 0xe000
#define ECHO_END 0xfdff

#define WRA0_BEGIN WRAM_BEGIN
#define WRA0_END 0xcfff
#define WRAX_BEGIN 0xd000
#define WRAX_END WRAM_END
#define MMIO_SVBK 0xff70

namespace gameboy {
    namespace wram {
        typedef std::array<u8, 0x1000> wram_bank_t;

        u8 dummy;

        std::array <wram_bank_t, 8> wram;

        u8 current_bank_idx = 1;

        void init() {
            srand(time(NULL));

            // Init all banks
            for (auto& b : wram) {
                for (auto& c : b) {
                    c = rand() % 0xff;
                }
            }
        }

        u32 read(u16 addr, size_t size) {
            if (addr == MMIO_SVBK) return settings::cgb_mode ? (0xf8 | current_bank_idx) : 0xff;

            if (addr >= WRA0_BEGIN && addr <= WRA0_END) {
                return utility::default_mb_read(wram[0].data(), addr, size, WRA0_BEGIN);
            }
                
            if (addr >= WRAX_BEGIN && addr <= WRAX_END) {
                return utility::default_mb_read(wram[(!current_bank_idx) ? 1 : current_bank_idx].data(), addr, size, WRAX_BEGIN);
            }
            return 0;
        }

        void write(u16 addr, u16 value, size_t size) {
            if (settings::cgb_mode) {
                if (addr == MMIO_SVBK) {
                    current_bank_idx = value & 0x7;
                    return;
                }
            }

            if (addr >= WRA0_BEGIN && addr <= WRA0_END) {
                utility::default_mb_write(wram[0].data(), addr, value, size, WRA0_BEGIN);
                return;
            }

            if (addr >= WRAX_BEGIN && addr <= WRAX_END) {
                utility::default_mb_write(wram[(!current_bank_idx) ? 1 : current_bank_idx].data(), addr, value, size, WRAX_BEGIN);
                return;
            }
        }

        u8& ref(u16 addr) {
            if (addr == MMIO_SVBK) return current_bank_idx;

            if (addr >= WRA0_BEGIN && addr <= WRA0_END) { return wram[0][addr-WRA0_BEGIN]; }
            if (addr >= WRAX_BEGIN && addr <= WRAX_END) { return wram[(!current_bank_idx) ? 1 : current_bank_idx].at(addr-WRAX_BEGIN); }
            return dummy;
        }
    }
}