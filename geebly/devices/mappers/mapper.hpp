#pragma once

#include "../../aliases.hpp"

#include <fstream>
#include <vector>
#include <array>

#define ROM_BEGIN   0x150
#define ROM_END     0x7fff
#define SRAM_BEGIN  0xa000
#define SRAM_END    0xbfff

namespace gameboy {
    namespace cart {
        enum class mapper_tag {
            rom_only,
            mbc1,
            mbc2,
            mbc3,
            mbc5,
            aec1,
            camera,
            no_cart = 0x100,
            unknown = 0x200
        };

        class mapper {
            u8 dummy = 0;
            
        public:
            mapper_tag tag;

            virtual u8* get_bank0() { return nullptr; };
            virtual u8* get_bank1() { return nullptr; };
            virtual u8* get_sram() { return nullptr; };
            virtual bool save_sram(std::ofstream& sav) { return false; };
            virtual void save_state(std::ofstream&) = 0;
            virtual void load_state(std::ifstream&) = 0;
            virtual int16_t get_sample() { return 0x0; }
            virtual inline bool vin_line_connected() { return false; }

            virtual void init(std::ifstream*) {};
            virtual u32 read(u16, size_t) { return 0; };
            virtual void write(u16, u16, size_t) {};
            virtual u8& ref(u16) { return dummy; };
        };
    }
}