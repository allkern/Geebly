#pragma once

#include <fstream>
#include <array>

#include "../aliases.hpp"
#include "../global.hpp"
#include "../log.hpp"

namespace gameboy {
    namespace bios {
        typedef std::array <u8, 0x900> big_bios_rom_t;

        u8 dummy;

        big_bios_rom_t rom;

        u32 read(u16 addr, size_t size) {
            return utility::default_mb_read(rom.data(), addr, size);
        }

        u8& ref(u16 addr) {
            return dummy;
        }

        void init(std::string filename) {
            std::ifstream f(filename, std::ios::binary);

            rom.fill(0xff);

            f.read((char*)rom.data(), rom.size());
        }
    }
}