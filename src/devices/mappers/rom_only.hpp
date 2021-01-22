#pragma once

#include "mapper.hpp"

#include "../../global.hpp"

namespace gameboy {
    namespace cart {
        class rom_only : public mapper {
            typedef std::array<u8, 0x7eb0> rom_t;

            rom_t rom = { 0 };

        public:
            u8* get_bank0() { return rom.data(); }
            u8* get_bank1() { return &rom[0x3eaf]; }

            void init(std::ifstream* f) override {
                tag = mapper_tag::rom_only;
                
                if (f->is_open() && f->good()) {
                    f->read((char*)rom.data(), rom.size());
                }
                
                f->close();
            }

            u32 read(u16 addr, size_t size) override {
                return utility::default_mb_read(rom.data(), addr, size, ROM_BEGIN);
            }
        };
    }
}
