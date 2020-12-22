#pragma once

#include <fstream>
#include <array>

#include "../aliases.hpp"
#include "../global.hpp"

namespace gameboy {
    namespace bios {
        typedef std::array <u8, 0x100> bios_rom_t;

        bios_rom_t rom;

        u32 read(u16 addr, size_t size) {
            return utility::default_mb_read(rom.data(), addr, size, 0);
        }

        u8& ref(u16 addr) {
            return rom[addr];
        }

        void init(std::string filename) {
            rom.fill(0);

            std::ifstream file(filename, std::ios::binary);

            file.read((char*)rom.data(), 0x100);
        }
    }
}