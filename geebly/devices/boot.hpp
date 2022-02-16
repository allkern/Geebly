#pragma once

#include <fstream>
#include <array>

#include "../aliases.hpp"
#include "../global.hpp"
#include "../log.hpp"

namespace gameboy {
    namespace boot {
        typedef std::array <u8, 0x900> big_bootrom_t;

        big_bootrom_t rom;

        u8 dummy;

        u32 read(u16 addr, size_t size) {
            return utility::default_mb_read(rom.data(), addr, size);
        }

        u8& ref(u16 addr) {
            return dummy;
        }

        void init(std::string filename) {
            std::ifstream f(filename, std::ios::binary);

            rom.fill(0xff);

            if (!f.is_open()) {
                _log(error, "Couldn't find bootrom \"%s\". Please specify another BIOS file, or enable skipping with \"-Bskip\"", filename.c_str());
                std::exit(1);
            }

            f.read((char*)rom.data(), rom.size());
        }
    }
}