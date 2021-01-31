#pragma once

#include <unordered_map>
#include <cstdlib>
#include <fstream>
#include <array>

#include "../aliases.hpp"
#include "../global.hpp"
#include "../log.hpp"

#include "mappers/no_cart.hpp"
#include "mappers/rom_only.hpp"
#include "mappers/mbc1.hpp"
#include "mappers/mbc2.hpp"
#include "mappers/mbc3.hpp"
#include "mappers/mbc5.hpp"

#define HDR_CART_TYPE 0x47
#define RVA_BEGIN 0x0
#define RVA_END 0xff
#define HDR_BEGIN 0x100
#define HDR_END 0x14f

namespace gameboy {
    namespace cart {
        std::string sav_name;

        typedef std::array<u8, 0x50> header_t;
        typedef std::array<u8, 0x100> rva_t;

        u8 dummy;

        rva_t rva;
        header_t header;

        mapper* cartridge = nullptr;

        void insert_cartridge(std::string rom) {
            if (rom == "geebly-no-cart") {
                rva.fill(0xff);
                header.fill(0xff);
                cartridge = new no_cart();

                return;
            }

            sav_name = rom.substr(0, rom.find_first_of('.')).append(".sav");

            std::ifstream f(rom, std::ios::binary);

            if (!f.is_open() || !f.good()) {
                #ifdef __linux__
                int i = system("zenity --error --width 300 --text \"Couldn't open ROM\"");
                #else
                _log(error, "Couldn't open ROM");
                #endif

                std::exit(1);
            }

            f.read((char*)rva.data(), rva.size());
            f.read((char*)header.data(), header.size());

            std::ifstream sav(sav_name);

            switch (header[HDR_CART_TYPE]) {
                case 0x00: { cartridge = new rom_only(); } break;
                case 0x01: { cartridge = new mbc1(sav); } break;
                case 0x02: { cartridge = new mbc1(sav); } break;
                case 0x03: { cartridge = new mbc1(sav); } break;
                // 0x04 unused
                case 0x05: { cartridge = new mbc2(sav); } break;
                case 0x06: { cartridge = new mbc2(sav); } break;
                // 0x07 unused
                case 0x08: { cartridge = new rom_only(); } break;
                case 0x09: { cartridge = new rom_only(); } break;
                // 0x0a unused
                // MMM01 unimplemented, probably MBC1-like?

                // MBC3 carts, RTC not supported
                case 0x0f: { cartridge = new mbc3(sav); } break;
                case 0x10: { cartridge = new mbc3(sav); } break;
                case 0x11: { cartridge = new mbc3(sav); } break;
                case 0x12: { cartridge = new mbc3(sav); } break;
                case 0x13: { cartridge = new mbc3(sav); } break;
                // 0x14-0x18 unused
                case 0x19: { cartridge = new mbc5(sav); } break;
                case 0x1a: { cartridge = new mbc5(sav); } break;
                case 0x1b: { cartridge = new mbc5(sav); } break;

                // Rumble carts, rumble not supported
                case 0x1c: { cartridge = new mbc5(sav); } break;
                case 0x1d: { cartridge = new mbc5(sav); } break;
                case 0x1e: { cartridge = new mbc5(sav); } break;

                // Pocket camera
                case 0xfc: { cartridge = new mbc1(sav); } break;
                // ...
                // Unsupported cartridge types:
                // - MMM01
                // - MBC6
                // - MBC7
                // - Pocket Camera
                // - Bandai TAMA5
                // - Hudson HuC3
                // - Hudson HuC1

                default: {
                    #ifdef __linux__
                    int i = system("zenity --error --width 300 --text \"Unimplemented cartridge type\"");
                    #else
                    _log(error, "Unimplemented cartridge type 0x%02x", header[HDR_CART_TYPE]);
                    #endif

                    std::exit(1);
                }
            }

            cartridge->init(&f);
        }

        void create_sav_file() {
            if (written_to_sram) {
                std::ofstream sav(sav_name, std::ios::binary);

                cartridge->save_sram(sav);
            }
        }

        u32 read(u16 addr, size_t size) {
            if (tilted_cartridge) return (u16)rand() % 0xffff;
            if (addr <= RVA_END) { return utility::default_mb_read(rva.data(), addr, size, RVA_BEGIN); }
            if (addr >= HDR_BEGIN && addr <= HDR_END) { return utility::default_mb_read(header.data(), addr, size, HDR_BEGIN); }
            if (addr >= ROM_BEGIN && addr <= ROM_END) { return cartridge->read(addr, size); }
            if (addr >= SRAM_BEGIN && addr <= SRAM_END) { return cartridge->read(addr, size); }
            return 0;
        }

        void write(u16 addr, u16 value, size_t size) {
            if (tilted_cartridge) { addr = (u16)rand() % 0xffff; value = (u16)rand() % 0xffff; }
            if (addr <= HDR_END) { return; }
            if (addr >= ROM_BEGIN && addr <= ROM_END) { cartridge->write(addr, value, size); return; }
            if (addr >= SRAM_BEGIN && addr <= SRAM_END) { written_to_sram = true; cartridge->write(addr, value, size); return; }
        }

        u8& ref(u16 addr) {
            if (tilted_cartridge) { addr = (u16)rand() % 0xffff; }
            if (addr >= SRAM_BEGIN && addr <= SRAM_END) { return cartridge->ref(addr); }
            return dummy;
        }

        u8* ptr(u16 addr) {
            if (tilted_cartridge) { addr = (u16)rand() % 0xffff; }
            if (addr <= RVA_END) { return &rva.at(addr); }
            if (addr >= HDR_BEGIN && addr <= HDR_END) { return &header.at(addr-HDR_BEGIN); }
            if (addr >= ROM_BEGIN && addr <= ROM_END) { return &cartridge->ref(addr); }
            if (addr >= SRAM_BEGIN && addr <= SRAM_END) { return &cartridge->ref(addr); }
            return nullptr;
        }
    }
}