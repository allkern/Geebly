#pragma once

#include "mapper.hpp"

#include "../../global.hpp"

namespace gameboy {
    namespace cart {
        class mbc5 : public mapper {
            typedef std::array <u8, 0x4000> rom_bank_t;
            typedef std::array <u8, 0x2000> sram_bank_t;
            typedef std::array <sram_bank_t, 16> sram_t;

            std::array <sram_bank_t, 16> sram;
            std::vector <rom_bank_t> rom;

            u16 current_rom_bank_idx = 0;

            rom_bank_t* current_rom_bank = nullptr;
            sram_bank_t* current_sram_bank = &sram[0];
            
            bool sram_enabled = true;

        public:
            mbc5(std::ifstream& sav) {
                if (sav.is_open()) {
                    for (sram_bank_t& b : sram) {
                        sav.read((char*)b.data(), b.size());
                    }
                    sav.close();
                }
            }

            u8 dummy;

            u8* get_bank0() { return rom[0].data(); }
            u8* get_bank1() { return current_rom_bank->data(); }
            u8* get_sram() { return current_sram_bank->data(); }
            
            void save_sram(std::ofstream& sav) override {
                if (sav.is_open()) {
                    for (sram_bank_t& b : sram) {
                        sav.write((char*)b.data(), 0x2000);
                    }
                }
                sav.close();
            }
            
            void init(std::ifstream* f) override {
                tag = mapper_tag::mbc1;
                
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
                if (addr >= 0xa000 && addr <= 0xbfff) { if (sram_enabled) return utility::default_mb_read(current_sram_bank->data(), addr, size, 0xa000); }
                
                return 0;
            }

            void write(u16 addr, u16 value, size_t size) override {
                if (addr >= 0xa000 && addr <= 0xbfff) {
                    if (sram_enabled) utility::default_mb_write(current_sram_bank->data(), addr, value, size, 0xa000);
                    return;
                }

                if (addr <= 0x1fff) {
                    sram_enabled = ((value & 0xf) == 0xa);
                }

                // SRAM bank select
                if (addr >= 0x4000 && addr <= 0x5fff) {
                    current_sram_bank = &sram[value & 0xf];
                }

                if (addr >= 0x2000 && addr <= 0x2fff) {
                    u16 old = current_rom_bank_idx;
                    current_rom_bank_idx = value & 0xff;
                    current_rom_bank_idx |= old & 0xff00;

                    // Selecting Bank 0 is supported on MBC5?
                    //if (!current_rom_bank_idx) current_rom_bank_idx++;

                    current_rom_bank = &rom[current_rom_bank_idx % rom.size()];
                }

                if (addr >= 0x3000 && addr <= 0x3fff) {
                    u16 old = current_rom_bank_idx;
                    current_rom_bank_idx = (value & 0xff) << 8;
                    current_rom_bank_idx |= old & 0xff;

                    // Selecting Bank 0 is supported on MBC5?
                    //if (!current_rom_bank_idx) current_rom_bank_idx++;

                    current_rom_bank = &rom[current_rom_bank_idx % rom.size()];
                }
            }

            u8& ref(u16 addr) {
                if (addr >= 0xa000 && addr <= 0xbfff) { return current_sram_bank->at(addr-0xa000); }

                return dummy;
            }
        };
    }
}