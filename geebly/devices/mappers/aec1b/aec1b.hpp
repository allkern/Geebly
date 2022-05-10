#pragma once

#include "../mapper.hpp"

#include "../../../global.hpp"
#include "ym3438.hpp"

#include "SDL_audio.h"

#include <cmath>

int16_t opn2_sample_buf = 0x0;

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
                OPN2_Clock(&ym3438, &opn2_sample_buf);

                return (ym3438.mol & 0x1ff) * 0x4fff;
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

                opn2_buffered_write(&ym3438, 1, 0x22, 0x00);
                opn2_buffered_write(&ym3438, 1, 0x27, 0x00);
                opn2_buffered_write(&ym3438, 1, 0x28, 0x00);
                opn2_buffered_write(&ym3438, 1, 0x28, 0x01);
                opn2_buffered_write(&ym3438, 1, 0x28, 0x02);
                opn2_buffered_write(&ym3438, 1, 0x28, 0x04);
                opn2_buffered_write(&ym3438, 1, 0x28, 0x05);
                opn2_buffered_write(&ym3438, 1, 0x28, 0x06);
                opn2_buffered_write(&ym3438, 1, 0x2b, 0x00);

                opn2_buffered_write(&ym3438, 1, 0x30, 0x71);
                opn2_buffered_write(&ym3438, 1, 0x34, 0x0d);
                opn2_buffered_write(&ym3438, 1, 0x38, 0x33);
                opn2_buffered_write(&ym3438, 1, 0x3c, 0x01);

                opn2_buffered_write(&ym3438, 1, 0x40, 0x23);
                opn2_buffered_write(&ym3438, 1, 0x44, 0x2d);
                opn2_buffered_write(&ym3438, 1, 0x48, 0x26);
                opn2_buffered_write(&ym3438, 1, 0x4c, 0x00);

                opn2_buffered_write(&ym3438, 1, 0x50, 0x5f);
                opn2_buffered_write(&ym3438, 1, 0x54, 0x99);
                opn2_buffered_write(&ym3438, 1, 0x58, 0x5f);
                opn2_buffered_write(&ym3438, 1, 0x5c, 0x94);

                opn2_buffered_write(&ym3438, 1, 0x60, 0x05);
                opn2_buffered_write(&ym3438, 1, 0x64, 0x05);
                opn2_buffered_write(&ym3438, 1, 0x68, 0x05);
                opn2_buffered_write(&ym3438, 1, 0x6c, 0x07);

                opn2_buffered_write(&ym3438, 1, 0x70, 0x02);
                opn2_buffered_write(&ym3438, 1, 0x74, 0x02);
                opn2_buffered_write(&ym3438, 1, 0x78, 0x02);
                opn2_buffered_write(&ym3438, 1, 0x7c, 0x02);

                opn2_buffered_write(&ym3438, 1, 0x80, 0x11);
                opn2_buffered_write(&ym3438, 1, 0x84, 0x11);
                opn2_buffered_write(&ym3438, 1, 0x88, 0x11);
                opn2_buffered_write(&ym3438, 1, 0x8c, 0xa6);

                opn2_buffered_write(&ym3438, 1, 0x90, 0x00);
                opn2_buffered_write(&ym3438, 1, 0x94, 0x00);
                opn2_buffered_write(&ym3438, 1, 0x98, 0x00);
                opn2_buffered_write(&ym3438, 1, 0x9c, 0x00);

                opn2_buffered_write(&ym3438, 1, 0xb0, 0x32);
                opn2_buffered_write(&ym3438, 1, 0xb4, 0xc0);
                opn2_buffered_write(&ym3438, 1, 0x28, 0x00);
                opn2_buffered_write(&ym3438, 1, 0xa4, 0x22);
                opn2_buffered_write(&ym3438, 1, 0xa0, 0x69);

                // Key on
                opn2_buffered_write(&ym3438, 1, 0x28, 0xf0);

                if (f->is_open() && f->good()) {
                    f->read((char*)rom.data(), rom.size());
                }
                
                f->close();
            }

            void write(u16 addr, u16 value, size_t size) override {
                // To-do
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