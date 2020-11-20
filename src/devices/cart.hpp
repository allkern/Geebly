#pragma once

#include <unordered_map>
#include <cstdlib>
#include <fstream>
#include <array>

#include "../aliases.hpp"
#include "../log.hpp"

#include "mappers/rom_only.hpp"
#include "mappers/mbc1.hpp"
#include "mappers/mbc3.hpp"

#define HDR_CART_TYPE 0x47
#define RVA_BEGIN 0x0
#define RVA_END 0xff
#define HDR_BEGIN 0x100
#define HDR_END 0x14f

namespace gameboy {
    namespace cart {
        typedef std::array<u8, 0x50> header_t;
        typedef std::array<u8, 0x100> rva_t;

        u8 dummy;

        rva_t rva;
        header_t header;

        mapper* cartridge = nullptr;

        void insert_cartridge(std::string rom) {
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

            switch (header[HDR_CART_TYPE]) {
                case 0x00: { cartridge = new rom_only(); } break;
                case 0x01: { cartridge = new mbc1(); } break;
                case 0x02: { cartridge = new mbc1(); } break;
                case 0x03: { cartridge = new mbc1(); } break;
                case 0x11: { cartridge = new mbc3(); } break;
                case 0x12: { cartridge = new mbc3(); } break;
                case 0x13: { cartridge = new mbc3(); } break;

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

        u32 read(u16 addr, size_t size) {
            if (addr <= RVA_END) { return utility::default_mb_read(rva.data(), addr, size, RVA_BEGIN); }
            if (addr >= HDR_BEGIN && addr <= HDR_END) { return utility::default_mb_read(header.data(), addr, size, HDR_BEGIN); }
            if (addr >= CART_ROM_BEGIN && addr <= CART_ROM_END) { return cartridge->read(addr, size); }
            if (addr >= CART_RAM_BEGIN && addr <= CART_RAM_END) { return cartridge->read(addr, size); }
            return 0;
        }

        void write(u16 addr, u16 value, size_t size) {
            if (addr <= HDR_END                               ) { return; }
            if (addr >= CART_ROM_BEGIN && addr <= CART_ROM_END) { cartridge->write(addr, value, size); return; }
            if (addr >= CART_RAM_BEGIN && addr <= CART_RAM_END) { cartridge->write(addr, value, size); return; }
        }


        u8& ref(u16 addr) {
            if (addr >= CART_RAM_BEGIN && addr <= CART_RAM_END) { return cartridge->ref(addr); }
            return dummy;
        }

        u8* ptr(u16 addr) {
            if (addr <= RVA_END) { return &rva.at(addr); }
            if (addr >= HDR_BEGIN && addr <= HDR_END) { return &header.at(addr-HDR_BEGIN); }
            if (addr >= CART_ROM_BEGIN && addr <= CART_ROM_END) { return &cartridge->ref(addr); }
            if (addr >= CART_RAM_BEGIN && addr <= CART_RAM_END) { return &cartridge->ref(addr); }
            return nullptr;
        }
    }
}