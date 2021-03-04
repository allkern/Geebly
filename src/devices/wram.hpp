#pragma once

#include <array>
#include <ctime>

#include "../aliases.hpp"
#include "../global.hpp"

#define WRAM_BEGIN 0xc000
#define WRAM_END 0xdfff

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

        wram_bank_t* current_bank = &wram[1];
        size_t current_bank_idx = 1;

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
                return utility::default_mb_read(current_bank->data(), addr, size, WRAX_BEGIN);
            }
            return 0;
        }

        void write(u16 addr, u16 value, size_t size) {
            if (settings::cgb_mode) {
                if (addr == MMIO_SVBK) {
                    current_bank_idx = value & 0x7;
                    current_bank = &wram[value == 0 ? 1 : current_bank_idx];
                    return;
                }
            }

            if (addr >= WRA0_BEGIN && addr <= WRA0_END) {
                utility::default_mb_write(wram[0].data(), addr, value, size, WRA0_BEGIN);
                return;
            }

            if (addr >= WRAX_BEGIN && addr <= WRAX_END) {
                utility::default_mb_write(current_bank->data(), addr, value, size, WRAX_BEGIN);
                return;
            }
        }

        u8& ref(u16 addr) {
            if (addr >= WRA0_BEGIN && addr <= WRA0_END) { return wram[0][addr-WRA0_BEGIN]; }
            if (addr >= WRAX_BEGIN && addr <= WRAX_END) { return current_bank->at(addr-WRAX_BEGIN); }
            return dummy;
        }
    }
}