#pragma once

#include "mapper.hpp"

#include "../../global.hpp"

namespace gameboy {
    namespace cart {
        class mbc2 : public mapper {
            typedef std::array <u8, 0x4000> rom_bank_t;
            typedef std::array <u8, 0x200> sram_t;

            sram_t sram;
            std::vector <rom_bank_t> rom;

            rom_bank_t* current_rom_bank = nullptr;
            
            bool sram_enabled = false;

        public:
            mbc2(std::ifstream& sav) {
                if (sav.is_open()) {
                    sav.read((char*)sram.data(), sram.size());
                    sav.close();
                }
            }

            u8 dummy;

            u8* get_bank0() { return rom[0].data(); }
            u8* get_bank1() { return current_rom_bank->data(); }
            u8* get_sram() { return sram.data(); }
            
            void init(std::ifstream* f) override {
                tag = mapper_tag::mbc2;
                
                if (f->is_open() && f->good()) {
                    f->seekg(0);
                    
                    rom_bank_t b = { 0 };

                    while (!f->eof()) {
                        f->read((char*)b.data(), b.size());
                        rom.push_back(b);
                        b.fill(0);
                    }

                    // Drop last bank, misread (fix?)
                    rom.pop_back();

                    current_rom_bank = &rom[1];
                }

                f->close();
            }

            u32 read(u16 addr, size_t size) override {
                if (addr >= 0x150 && addr <= 0x3fff) { return utility::default_mb_read(rom[0].data(), addr, size, 0); }
                if (addr >= 0x4000 && addr <= 0x7fff) { return utility::default_mb_read(current_rom_bank->data(), addr, size, 0x4000); }
                if (addr >= 0xa000 && addr <= 0xa1ff) { if (sram_enabled) return 0xf0 | (utility::default_mb_read(sram.data(), addr, size, 0xa000) & 0xf); }
                
                return 0;
            }

            void write(u16 addr, u16 value, size_t size) override {
                if (addr >= 0xa000 && addr <= 0xa1ff) {
                    if (sram_enabled) utility::default_mb_write(sram.data(), addr, value & 0xf, size, 0xa000);
                    return;
                }

                if (!(addr & 0x100)) sram_enabled = ((value & 0xf) == 0xa);

                if (addr & 0x100) {
                    if ((value & 0xf) == 0x0) value++;

                    current_rom_bank = &rom[(value & 0xf) % rom.size()];
                }
            }

            u8& ref(u16 addr) {
                if (addr >= 0xa000 && addr <= 0xa1ff) { return sram.at(addr-0xa000); }

                return dummy;
            }
        };
    }
}