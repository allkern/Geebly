#pragma once

#include "aliases.hpp"
#include "global.hpp"

#include "cpu/cpu.hpp"
#include "cpu/mnemonics.hpp"
#include "log.hpp"
#include "cli.hpp"

#include <fstream>

namespace gameboy {
    bool file_exists(std::string name) {
        std::ifstream f(name);

        return f.good() && f.is_open();
    }

    // To-do: Make ROM dropping better
    void reset() {
        spu::reset();
        wram::init();
        cpu::init();
        bus::init();
        hram::init();
    }

    void reload_rom(const char* file) {
        reset();

        cart::insert_cartridge(file);
    }

    void init(std::string rom_reload = "") {
        bool bootrom_present = false;

        if (settings::cgb_mode) {
            bootrom_present =
                file_exists("cgb_boot.bin") ||
                file_exists("boot/cgb_boot.bin");
        } else {
            bootrom_present =
                file_exists("dmg_boot.bin") ||
                file_exists("boot/dmg_boot.bin");
        }

        if (!bootrom_present)
            _log(warning, "Couldn't find any DMG or CGB boot ROMs, skipping boot logo");

        settings::skip_bootrom |= bootrom_present;
 
        if (!settings::skip_bootrom)
            bios::init(cli::setting("boot", settings::cgb_mode ? "cgb_boot.bin" : "dmg_boot.bin"));

        if (!cli::is_defined("cartridge"))
            _log(warning, "No cartridge inserted, loading no_cart");

        cart::insert_cartridge(cli::setting("cartridge", "geebly-no-cart"));
        wram::init();
        hram::init();
        cpu::init();
        bus::init();
        clock::init(cpu::registers::last_instruction_cycles);

        cpu::handle_interrupts();
        cpu::fetch();

        pause = settings::debugger_enabled;
        step = true;

        if (!settings::bios_checks_enabled) {
            bios::rom[0xfa] = 0x00;
            bios::rom[0xfb] = 0x00;
            bios::rom[0xe9] = 0x00;
            bios::rom[0xea] = 0x00;
        }

        if (!sound_disabled) spu::init();
    }

    void update() {
        if (!pause) {
            cpu::cycle();

            if (!cpu::stopped) {
                ppu::cycle();
                timer::update();
            }

            if (settings::enable_joyp_irq_delay) joypad::update();
        } else {
            if (step) {
                cpu::execute();
                cpu::handle_interrupts();
                cpu::fetch();

                if (!cpu::stopped) {
                    ppu::cycle();
                    timer::update();
                }

                if (settings::enable_joyp_irq_delay) joypad::update();

                step = false;
            }
        }
    }
}