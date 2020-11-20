#pragma once

#include "../../aliases.hpp"

#include <fstream>
#include <vector>
#include <array>

#define CART_ROM_BEGIN 0x150
#define CART_ROM_END 0x7fff
#define CART_RAM_BEGIN 0xa000
#define CART_RAM_END 0xbfff

namespace gameboy {
    namespace cart {
        enum class mapper_tag {
            rom_only,
            mbc1,
            mbc2,
            mbc3
        };

        class mapper {
            u8 dummy = 0;
            
        public:
            mapper_tag tag;

            virtual void init(std::ifstream*) {};
            virtual u32 read(u16, size_t) { return 0; };
            virtual void write(u16, u16, size_t) {};
            virtual u8& ref(u16) { return dummy; };
        };
    }
}