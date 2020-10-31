#pragma once

#include "../aliases.hpp"

#include <fstream>
#include <array>

#define CART_ROM_BEGIN 0x150
#define CART_ROM_END 0x7fff
#define CART_RAM_BEGIN 0xa000
#define CART_RAM_END 0xbfff

namespace gameboy {
    namespace cart {
        class mapper {
            u8 ro_sink = 0;
            
        public:
            virtual void init(std::ifstream*) {};
            virtual u32 read(u16, size_t) const { return 0; };
            virtual void write(u16, u16, size_t) {};
            virtual u8& ref(u16) { return ro_sink; };
        };

        class rom_only : public mapper {
            typedef std::array<u8, 0x7eaf> rom_t;

            rom_t rom = { 0 };

        public:
            void init(std::ifstream* f) {
                if (f->is_open() && f->good()) {
                    f->read((char*)rom.data(), rom.size() + 1);
                }
                
                f->close();
            }

            u32 read(u16 addr, size_t size) const override {
                u32 d = 0;
                while (size) {
                    d |= rom[(addr-CART_ROM_BEGIN)+(size-1)] << (((size--)-1)*8);
                }
                return d;
            }
        };
    }
}