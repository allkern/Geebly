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
#include "mappers/camera.hpp"

#define HDR_CGB_COMPATIBLE 0x43
#define HDR_CART_TYPE      0x47
#define HDR_TITLE          0x34
#define HDR_ENTRY          0x00
#define HDR_MANUFACTURER   0x3f

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

        static std::unordered_map <u8, const char*> cart_type_map = {
            { 0x00, "ROM ONLY" },
            { 0x01, "MBC1" },
            { 0x02, "MBC1+RAM" },
            { 0x03, "MBC1+RAM+BATTERY" },
            { 0x05, "MBC2" },
            { 0x06, "MBC2+BATTERY" },
            { 0x08, "ROM+RAM" },
            { 0x09, "ROM+RAM+BATTERY" },
            { 0x0b, "MMM01" },
            { 0x0c, "MMM01+RAM" },
            { 0x0d, "MMM01+RAM+BATTERY" },
            { 0x0f, "MBC3+RTC+BATTERY" },
            { 0x10, "MBC3+RTC+RAM+BATTERY" },
            { 0x11, "MBC3" },
            { 0x12, "MBC3+RAM" },
            { 0x13, "MBC3+RAM+BATTERY" },
            { 0x19, "MBC5" },
            { 0x1a, "MBC5+RAM" },
            { 0x1b, "MBC5+RAM+BATTERY" },
            { 0x1c, "MBC5+RUMBLE" },
            { 0x1d, "MBC5+RUMBLE+RAM" },
            { 0x1e, "MBC5+RUMBLE+RAM+BATTERY" },
            { 0x20, "MBC6" },
            { 0x22, "MBC7+SENSOR+RUMBLE+RAM+BATTERY" },
            { 0xfc, "POCKET CAMERA" },
            { 0xfd, "BANDAI TAMA5" },
            { 0xfe, "HuC3" },
            { 0xff, "HuC1+RAM+BATTERY" }
        };

        static std::unordered_map <u8, const char*> rom_size_map = {
            { 0x00, "32 KB" },
            { 0x01, "64 KB" },
            { 0x02, "128 KB" },
            { 0x03, "256 KB" },
            { 0x04, "512 KB" },
            { 0x05, "1 MB" },
            { 0x06, "2 MB" },
            { 0x07, "4 MB" },
            { 0x08, "8 MB" },
            { 0x52, "1.1 MB" },
            { 0x53, "1.2 MB" },
            { 0x54, "1.5 MB" }
        };

        static std::unordered_map <u8, const char*> sram_size_map = {
            { 0x00, "No SRAM present or MBC2 On-board" },
            { 0x01, "Unused (Possibly PD ROM)" },
            { 0x02, "8 KB" },
            { 0x03, "32 KB" },
            { 0x04, "128 KB" },
            { 0x05, "64 KB" }
        };

        static std::unordered_map <u8, u8> sram_bc_map = {
            { 0x00, 0 },
            { 0x01, 0 },
            { 0x02, 1 },
            { 0x03, 4 },
            { 0x04, 16 },
            { 0x05, 8 }
        };

        static std::unordered_map <u8, const char*> licensee_code_map = {
            { 0x00, "None" },
            { 0x01, "Nintendo R&D1" },
            { 0x08, "Capcom" },
            { 0x13, "Electronic Arts" },
            { 0x18, "Hudson Soft" },
            { 0x19, "b-ai" },
            { 0x20, "kss" },
            { 0x22, "pow" },
            { 0x24, "PCM Complete" },
            { 0x25, "san-x" },
            { 0x28, "Kemco Japan" },
            { 0x29, "seta" },
            { 0x30, "Viacom" },
            { 0x31, "Nintendo" },
            { 0x32, "Bandai" },
            { 0x33, "Ocean/Acclaim" },
            { 0x34, "Konami" },
            { 0x35, "Hector" },
            { 0x37, "Taito" },
            { 0x38, "Hudson" },
            { 0x39, "Banpresto" },
            { 0x41, "Ubi Soft" },
            { 0x42, "Atlus" },
            { 0x44, "Malibu" },
            { 0x46, "angel" },
            { 0x47, "Bullet-Proof" },
            { 0x49, "irem" },
            { 0x50, "Absolute" },
            { 0x51, "Acclaim" },
            { 0x52, "Activision" },
            { 0x53, "American sammy" },
            { 0x54, "Konami" },
            { 0x55, "Hi tech entertainment" },
            { 0x56, "LJN" },
            { 0x57, "Matchbox" },
            { 0x58, "Mattel" },
            { 0x59, "Milton Bradley" },
            { 0x60, "Titus" },
            { 0x61, "Virgin" },
            { 0x64, "LucasArts" },
            { 0x67, "Ocean" },
            { 0x69, "Electronic Arts" },
            { 0x70, "Infogrames" },
            { 0x71, "Interplay" },
            { 0x72, "Broderbund" },
            { 0x73, "sculptured" },
            { 0x75, "sci" },
            { 0x78, "THQ" },
            { 0x79, "Accolade" },
            { 0x80, "misawa" },
            { 0x83, "lozc" },
            { 0x86, "Tokuma Shoten Intermedia" },
            { 0x87, "Tsukuda Original" },
            { 0x91, "Chunsoft" },
            { 0x92, "Video system" },
            { 0x93, "Ocean/Acclaim" },
            { 0x95, "Varie" },
            { 0x96, "Yonezawa/s'pal" },
            { 0x97, "Kaneko" },
            { 0x99, "Pack in soft" },
            { 0xa4, "Konami (Yu-Gi-Oh!)" }
        };

        struct info_t {
            u8   entry[4],             // 0x100-0x103
                 logo[0x30];           // 0x104-0x133
            char title[16],            // 0x134-0x143
                 new_licensee_code[2]; // 0x144-0x145
            u8   sgb_flag,             // 0x146
                 type,                 // 0x147
                 rom_size,             // 0x148
                 sram_size,            // 0x149
                 destination_code,     // 0x14a
                 legacy_licensee_code, // 0x14b
                 rom_version,          // 0x14c
                 checksum;             // 0x14d
            u16  global_checksum;      // 0x14e-0x14f
        };

        struct settings_t {
            mapper_tag  type;
            bool        sram_present,
                        sram_battery_backed;
        };

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

            info_t cart_info = *reinterpret_cast<info_t*>(&header[HDR_ENTRY]);

            std::ifstream sav(sav_name);

            // WIP
            //static const std::unordered_map <u8, cartridge_settings_t> settings_map = {
            //    { 0x00, { mapper_tag::no_cart   , false, false } },
            //    { 0x01, { mapper_tag::mbc1      , false, false } },
            //    { 0x02, { mapper_tag::mbc1      , true , false } },
            //    { 0x03, { mapper_tag::mbc1      , true , true  } },
            //    { 0x05, { mapper_tag::mbc2      , true , false } },
            //    { 0x06, { mapper_tag::mbc2      , true , true  } }
            //};

            //if (!(header[HDR_CGB_COMPATIBLE] == 0x80 || header[HDR_CGB_COMPATIBLE] == 0xc0)) settings::cgb_mode = false;

            _log(info, "Cartridge info:\n"
                "\tTitle: %.16s\n"
                "\tNew Licensee Code: %.2s\n"
                "\tSGB Enhanced: %s (%02x)\n"
                "\tCartridge Type: %s (%02x)\n"
                "\tROM Size: %s (%02x)\n"
                "\tSRAM Size: %s (%02x)\n"
                "\tDestination Code: %s (%02x)\n"
                "\tLicensee Code: %s (%02x)\n"
                "\tROM Version: %02x\n"
                "\tChecksum: 0x%02x\n"
                "\tGlobal Checksum: 0x%04x",
                cart_info.title,
                cart_info.new_licensee_code,
                (cart_info.sgb_flag == 0x03) ? "Yes" : "No", cart_info.sgb_flag,
                cart_type_map[cart_info.type], cart_info.type,
                rom_size_map[cart_info.rom_size], cart_info.rom_size,
                sram_size_map[cart_info.sram_size], cart_info.sram_size,
                (cart_info.destination_code == 0x01) ? "Japanese" : "Non-Japanese", cart_info.destination_code,
                licensee_code_map[cart_info.legacy_licensee_code], cart_info.legacy_licensee_code,
                cart_info.rom_version,
                cart_info.checksum,
                cart_info.global_checksum
            );

            u8 sbc = sram_bc_map[cart_info.sram_size];

            switch (header[HDR_CART_TYPE]) {
                case 0x00: { cartridge = new rom_only();           } break;
                case 0x01: { cartridge = new mbc1();               } break;
                case 0x02: { cartridge = new mbc1(true);           } break;
                case 0x03: { cartridge = new mbc1(true, &sav);     } break;
                // 0x04 unused
                case 0x05: { cartridge = new mbc2();               } break;
                case 0x06: { cartridge = new mbc2(&sav);           } break;
                // 0x07 unused
                case 0x08: { cartridge = new rom_only(true);       } break;
                case 0x09: { cartridge = new rom_only(true, &sav); } break;
                // 0x0a unused
                // MMM01 unimplemented, probably MBC1-like?

                // MBC3 carts, RTC not supported
                case 0x0f: { cartridge = new mbc3(sbc);            } break;
                case 0x10: { cartridge = new mbc3(sbc, true, &sav);} break;
                case 0x11: { cartridge = new mbc3(sbc);            } break;
                case 0x12: { cartridge = new mbc3(sbc, true);      } break;
                case 0x13: { cartridge = new mbc3(sbc, true, &sav);} break;
                // 0x14-0x18 unused
                case 0x19: { cartridge = new mbc5();               } break;
                case 0x1a: { cartridge = new mbc5(true);           } break;
                case 0x1b: { cartridge = new mbc5(true, &sav);     } break;

                // Rumble carts, rumble not supported
                case 0x1c: { cartridge = new mbc5();               } break;
                case 0x1d: { cartridge = new mbc5(true);           } break;
                case 0x1e: { cartridge = new mbc5(true, &sav);     } break;

                // Pocket camera
                case 0xfc: { cartridge = new camera(sbc, true, &sav);} break;
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

            _log(ok, "Loaded cartridge");
            cartridge->init(&f);
            _log(ok, "Initialized cartridge");
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