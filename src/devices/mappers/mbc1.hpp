#pragma once

#include "mapper.hpp"

#include "../../global.hpp"

namespace gameboy {
    namespace cart {
        class mbc1 : public mapper {
            enum mode : bool {
                big_rom = false,
                big_sram = true
            };

            typedef std::array <u8, 0x4000> rom_bank_t;
            typedef std::array <u8, 0x2000> sram_bank_t;
            typedef std::array <sram_bank_t, 4> sram_t;

            std::array <sram_bank_t, 4> sram;
            std::vector <rom_bank_t> rom;

            u8 current_rom_bank_idx = 0;

            rom_bank_t* current_rom_bank = nullptr;
            sram_bank_t* current_sram_bank = &sram[0];  
            
            // SRAM settings
            bool sram_enabled = true,
                 sram_present = false,
                 sram_battery_backed = false;

            bool mode = mode::big_rom;

        public:
            mbc1(bool has_sram = false, std::ifstream* sav = nullptr) {
                sram_present = has_sram;
                sram_battery_backed = sram_present && (sav != nullptr);

                if (!sram_battery_backed) return;

                if (sav->is_open()) {
                    for (sram_bank_t& b : sram) {
                        sav->read((char*)b.data(), b.size());
                    }
                    sav->close();
                }
            }

            u8 dummy = 0;

            u8* get_bank0() override { return rom[0].data(); }
            u8* get_bank1() override { return current_rom_bank->data(); }
            u8* get_sram() override { return current_sram_bank->data(); }

            bool save_sram(std::ofstream& sav) override {
                if (!sram_battery_backed) return false;

                if (sav.is_open()) {
                    for (sram_bank_t& b : sram) {
                        sav.write((char*)b.data(), b.size());
                    }
                }
                sav.close();

                return true;
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
                if (addr >= 0x150 && addr <= 0x3fff) { return utility::default_mb_read(rom[0].data(), addr, size); }
                if (addr >= 0x4000 && addr <= 0x7fff) { return utility::default_mb_read(current_rom_bank->data(), addr, size, 0x4000); }
                
                if (sram_present)
                    if (addr >= 0xa000 && addr <= 0xbfff)
                        if (sram_enabled) return utility::default_mb_read(current_sram_bank->data(), addr, size, 0xa000);   
                
                return 0xff;
            }

            void write(u16 addr, u16 value, size_t size) override {
                if (sram_present) {
                    if (addr >= 0xa000 && addr <= 0xbfff) {
                        if (sram_enabled)
                            utility::default_mb_write(current_sram_bank->data(), addr, value, size, 0xa000);
                        return;
                    }
                }

                if (addr <= 0x1fff) sram_enabled = ((value & 0xf) == 0xa);

                // SRAM bank select/ROM MSB select
                if (addr >= 0x4000 && addr <= 0x5fff) {
                    if (mode == mode::big_rom) {
                        u8 old = current_rom_bank_idx;
                        current_rom_bank_idx = (value & 0x3) << 5;
                        current_rom_bank_idx |= old & 0x1f;
                        current_rom_bank = &rom[current_rom_bank_idx % rom.size()];
                    }

                    if (mode == mode::big_sram) {
                        current_sram_bank = &sram[value & 0x3];
                    }
                }

                // Mode select
                if (addr >= 0x6000 && addr <= 0x7fff) {
                    mode = value;
                    if (mode) {
                        current_rom_bank_idx &= 0x1f;
                        current_rom_bank = &rom[current_rom_bank_idx];
                    } else {
                        // Reset SRAM bank to bank 0 when going from mode 1 to mode 0
                        current_sram_bank = &sram[0];
                    }
                    return;
                }

                if (addr >= 0x2000 && addr <= 0x3fff) {
                    if ((value & 0x1f) == 0x0) value++;

                    u8 old = current_rom_bank_idx;
                    current_rom_bank_idx = value;
                    current_rom_bank_idx |= old & 0x60;
               
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