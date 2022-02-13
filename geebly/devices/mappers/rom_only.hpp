#pragma once

#include "mapper.hpp"

#include "../../global.hpp"

namespace gameboy {
    namespace cart {
        class rom_only : public mapper {
            u8 dummy = 0;

            typedef std::array<u8, 0x7eb0> rom_t;
            typedef std::array<u8, 0x2000> sram_t;

            rom_t rom = { 0 };
            sram_t sram = { 0 };

            bool sram_present = false,
                 sram_battery_backed = false;

        public:
            void save_state(std::ofstream& o) override {
                o.write(reinterpret_cast<char*>(sram.data()), sram.size());
            }

            void load_state(std::ifstream& i) override {
                i.read(reinterpret_cast<char*>(sram.data()), sram.size());
            }

            rom_only(bool has_sram = false, std::ifstream* sav = nullptr) {
                sram_present = has_sram;
                sram_battery_backed = sram_present && (sav != nullptr);

                if (!sram_battery_backed) return;

                if (sav->is_open()) {
                    sav->read((char*)sram.data(), sram.size());
                    sav->close();
                }
            }

            u8* get_bank0() { return rom.data(); }
            u8* get_bank1() { return &rom[0x3eaf]; }
            u8* get_sram() { return sram.data(); }

            bool save_sram(std::ofstream& sav) override {
                if (!sram_battery_backed) return false;

                if (sav.is_open()) {
                    sav.write((char*)sram.data(), sram.size());
                }
                sav.close();

                return true;
            }

            void init(std::ifstream* f) override {
                tag = mapper_tag::rom_only;
                
                if (f->is_open() && f->good()) {
                    f->read((char*)rom.data(), rom.size());
                }
                
                f->close();
            }

            u32 read(u16 addr, size_t size) override {
                if (addr >= 0x150 && addr <= 0x7fff) { return utility::default_mb_read(rom.data(), addr, size, ROM_BEGIN); }

                return 0xff;
            }

            u8& ref(u16 addr) override {
                if (addr >= 0x150 && addr <= 0x7fff) { return rom.at(addr-0x150); }
                if (addr >= 0xa000 && addr <= 0xbfff) { return sram.at(addr-0xa000); }

                return dummy;
            }
        };
    }
}
