#pragma once

#include "../aliases.hpp"

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
            mbc1
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

        class rom_only : public mapper {
            typedef std::array<u8, 0x7eb0> rom_t;

            rom_t rom = { 0 };

        public:
            void init(std::ifstream* f) override {
                tag = mapper_tag::rom_only;
                
                if (f->is_open() && f->good()) {
                    f->read((char*)rom.data(), rom.size());
                }
                
                f->close();
            }

            u32 read(u16 addr, size_t size) override {
                return utility::default_mb_read(rom.data(), addr, size, CART_ROM_BEGIN);
            }
        };


        class mbc1 : public mapper {
            typedef std::array <u8, 0x4000> bank;

            std::vector <bank> banks;

            bank* current_bank = nullptr;
            
            bool ram_enabled = false;

        public:
            void init(std::ifstream* f) override {
                tag = mapper_tag::mbc1;
                
                if (f->is_open() && f->good()) {
                    f->seekg(0);
                    
                    bank b = { 0 };

                    while (!f->eof()) {
                        f->read((char*)b.data(), b.size());
                        banks.push_back(b);
                        b.fill(0);
                    }

                    // Drop last bank, invalid (fix?)
                    banks.pop_back();

                    current_bank = &banks[1];
                }

                f->close();
            }

            u32 read(u16 addr, size_t size) override {
                if (addr >= 0x150 && addr <= 0x3fff) { return utility::default_mb_read(banks[0].data(), addr, size, 0); }
                if (addr >= 0x4000 && addr <= 0x7fff) { return utility::default_mb_read(current_bank->data(), addr, size, 0x4000); }
                
                return 0;
            }

            void write(u16 addr, u16 value, size_t size) override {
                if (addr <= 0x1fff) {
                    if (value == 0x0a) { ram_enabled = true; }
                    if (value == 0x00) { ram_enabled = false; }
                }

                if (addr >= 0x2000 && addr <= 0x3fff) {
                    if ((value & 0x1f) == 0x0) { value++; }

                    current_bank = &banks[(value) % banks.size()];
                }
            }
        };
    }
}