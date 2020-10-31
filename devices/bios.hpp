#pragma once

#include <fstream>
#include <array>

#include "../aliases.hpp"

namespace gameboy {
    namespace bios {
        typedef std::array <u8, 0xff> bios_rom_t;

        bios_rom_t rom;

        u32 read(u16 addr, size_t size) {
            u32 r = 0; size_t s = size;
            while (size) {
                r |= rom[addr++] << ((s - (size--))*8);
            }
            return r;
        }

        u8& ref(u16 addr) {
            return rom[addr];
        }

        void init(std::string filename) {
            rom.fill(0);

            std::ifstream file(filename, std::ios::binary);

            file.read((char*)rom.data(), 0xff);
        }
    }
}