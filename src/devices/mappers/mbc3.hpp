#pragma once

#include "mapper.hpp"

namespace gameboy {
    namespace cart {
        class mbc3 : public mapper {
            typedef std::array <u8, 0x4000> bank;
            typedef std::array <u8, 0x2000> ram_t;


            ram_t ram = { 0 };

            std::vector <bank> banks;

            bank* current_bank = nullptr;
            
            bool ram_enabled = false;

        public:
            u8 dummy;
            
            void init(std::ifstream* f) override {
                tag = mapper_tag::mbc3;
                
                if (f->is_open() && f->good()) {
                    f->seekg(0);
                    
                    bank b = { 0 };

                    while (!f->eof()) {
                        f->read((char*)b.data(), b.size());
                        banks.push_back(b);
                        b.fill(0);
                    }

                    // Drop last bank, misread (fix?)
                    banks.pop_back();

                    current_bank = &banks[1];
                }

                f->close();
            }

            u32 read(u16 addr, size_t size) override {
                if (addr >= 0x150 && addr <= 0x3fff) { return utility::default_mb_read(banks[0].data(), addr, size, 0); }
                if (addr >= 0x4000 && addr <= 0x7fff) { return utility::default_mb_read(current_bank->data(), addr, size, 0x4000); }
                if (addr >= 0xa000 && addr <= 0xbfff) { return utility::default_mb_read(ram.data(), addr, size, 0xa000); }
                
                return 0;
            }

            void write(u16 addr, u16 value, size_t size) override {
                if (addr >= 0xa000 && addr <= 0xbfff) { utility::default_mb_write(ram.data(), addr, value, size, 0xa000); return; }

                if (addr <= 0x1fff) {
                    if (value == 0x0a) { ram_enabled = true; }
                    if (value == 0x00) { ram_enabled = false; }
                }

                if (addr >= 0x2000 && addr <= 0x3fff) {
                    if ((value & 0x1f) == 0x0) { value++; }

                    current_bank = &banks[(value) % banks.size()];
                }
            }

            u8& ref(u16 addr) {
                if (addr >= 0x150 && addr <= 0x3fff) { return banks[0].at(addr); }
                if (addr >= 0x4000 && addr <= 0x7fff) { return current_bank->at(addr); }
                if (addr >= 0xa000 && addr <= 0xbfff) { ram.at(addr); }

                return dummy;
            }
        };
    }
}