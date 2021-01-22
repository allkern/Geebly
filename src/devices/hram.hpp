#pragma once

#include <array>
#include <ctime>

#include "../aliases.hpp"
#include "../global.hpp"

#define HRAM_BEGIN 0xff80
#define HRAM_END 0xfffe

namespace gameboy {
    namespace hram {
        typedef std::array<u8, 0x80> hram_t;

        u8 dummy;

        hram_t hram;

        void init() {
            srand(time(NULL));

            for (auto& b : hram) { b = rand() % 0xff; }
        }

        u32 read(u16 addr, size_t size) {
            if (addr >= HRAM_BEGIN && addr <= HRAM_END) {
                return utility::default_mb_read(hram.data(), addr, size, HRAM_BEGIN);
            }
            return 0;
        }

        void write(u16 addr, u16 value, size_t size) {
            if (addr >= HRAM_BEGIN && addr <= HRAM_END) {
                utility::default_mb_write(hram.data(), addr, value, size, HRAM_BEGIN);
                return;
            }
        }

        u8& ref(u16 addr) {
            if (addr >= HRAM_BEGIN && addr <= HRAM_END) { return hram[addr-HRAM_BEGIN]; }
            return dummy;
        }
    }
}