#pragma once

#include "../mapper.hpp"

#include "../../../global.hpp"
#include "ym3438.hpp"

#include <cmath>

namespace gameboy {
    namespace cart {
        class aec1b : public mapper {
            u8 dummy = 0;

            typedef std::array<u8, 0x7eb0> rom_t;

            ym3438_t ym3438;

            rom_t rom = { 0 };

        public:
            inline bool vin_line_connected() override {
                return true;
            }

            int16_t get_sample() override {
                int16_t buf;

                OPN2_Clock(&ym3438, &buf);

                return buf & 0x1ff;
            }

            void save_state(std::ofstream& o) override {}
            void load_state(std::ifstream& i) override {}

            aec1b(bool has_sram = false, std::ifstream* sav = nullptr) {}

            u8* get_bank0() { return rom.data(); }
            u8* get_bank1() { return &rom[0x3eaf]; }
            u8* get_sram() { return nullptr; }

            bool save_sram(std::ofstream& sav) override {
                return false;
            }

            void init(std::ifstream* f) override {
                tag = mapper_tag::aec1b;

                OPN2_Reset(&ym3438);

                if (f->is_open() && f->good()) {
                    f->read((char*)rom.data(), rom.size());
                }
                
                f->close();
            }

            void write(u16 addr, u16 value, size_t size) override {
            };

            u32 read(u16 addr, size_t size) override {
                if (addr >= 0x150 && addr <= 0x7fff) { return utility::default_mb_read(rom.data(), addr, size, ROM_BEGIN); }
                return 0xff;
            }

            u8& ref(u16 addr) override {
                if (addr >= 0x150 && addr <= 0x7fff) { return rom.at(addr-0x150); }

                return dummy;
            }
        };
    }
}

#undef SPU_NATIVE_SAMPLERATE
#undef M_PI