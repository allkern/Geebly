#pragma once

#include "mapper.hpp"

#include "../../global.hpp"

namespace gameboy {
    namespace cart {
        class mbc3 : public mapper {
            typedef std::array <u8, 0x4000> rom_bank_t;
            typedef std::array <u8, 0x2000> sram_bank_t;
            typedef std::array <sram_bank_t, 8> sram_t;

            std::array <sram_bank_t, 8> sram;
            std::vector <rom_bank_t> rom;

            u8 current_rom_bank_idx = 0, sram_bank_count = 0;

            rom_bank_t* current_rom_bank = nullptr;
            sram_bank_t* current_sram_bank = &sram[0];
            
            bool sram_enabled = true,
                 sram_present = false,
                 sram_battery_backed = false;

        public:
            mbc3(u8 sram_bc = 0, bool has_sram = false, std::ifstream* sav = nullptr) {
                sram_present = has_sram;
                sram_battery_backed = sram_present && (sav != nullptr);
                sram_bank_count = sram_bc;

                if (!sram_battery_backed) return;

                if (sav->is_open()) {
                    for (sram_bank_t& b : sram) {
                        if (!(sram_bc--)) break;

                        sav->read((char*)b.data(), b.size());
                    }

                    sav->close();
                }
            }

            u8 dummy;

            u8* get_bank0() { return rom[0].data(); }
            u8* get_bank1() { return current_rom_bank->data(); }
            u8* get_sram() { return current_sram_bank->data(); }
            
            bool save_sram(std::ofstream& sav) override {
                if (!sram_battery_backed) return false;

                if (sav.is_open()) {
                    for (sram_bank_t& b : sram) {
                        if (!(sram_bank_count--)) break;

                        sav.write((char*)b.data(), b.size());
                    }
                }

                sav.close();

                return true;
            }
            
            void init(std::ifstream* f) override {
                tag = mapper_tag::mbc3;
                
                if (f->is_open() && f->good()) {
                    f->seekg(0);
                    
                    rom_bank_t b = { 0 };

                    while (!f->eof()) {
                        f->read((char*)b.data(), b.size());
                        rom.push_back(b);
                        b.fill(0xff);
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

                if (addr <= 0x1fff) { sram_enabled = ((value & 0xf) == 0xa); return; }

                // SRAM bank select/ROM MSB select
                if (addr >= 0x4000 && addr <= 0x5fff) {
                    current_sram_bank = &sram[value & 0x3];
                    return;
                }

                if (addr >= 0x2000 && addr <= 0x3fff) {
                    if ((value & 0x7f) == 0x0) value++;

                    current_rom_bank_idx = value;

                    current_rom_bank = &rom[current_rom_bank_idx % rom.size()];
                }
            }

            u8& ref(u16 addr) {
                if (addr >= 0x150 && addr <= 0x3fff) { return rom[0].at(addr-0x150); }
                if (addr >= 0x4000 && addr <= 0x7fff) { return current_rom_bank->at(addr-0x4000); }
                if (addr >= 0xa000 && addr <= 0xbfff) { return current_sram_bank->at(addr-0xa000); }

                return dummy;
            }
        };
    }
}