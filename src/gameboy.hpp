#pragma once

#include "aliases.hpp"
#include "global.hpp"

#include "cpu/cpu.hpp"
#include "cpu/mnemonics.hpp"
#include "log.hpp"
#include "cli.hpp"

namespace gameboy {
    // To-do: Make ROM dropping better
    void reset() {
        spu::reset();
        wram::init();
        cpu::init();
        bus::init();
        hram::init();
    }

    void reload_rom(std::string file) {
        reset();

        cart::insert_cartridge(file);
    }

    void init(std::string rom_reload = "") {
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

        ppu::init();
        screen::init(&ppu::frame, std::stoi(cli::setting("scale", "1")));
        screen::register_keydown_cb(joypad::keydown);
        screen::register_keyup_cb(joypad::keyup);
        screen::register_rom_dropped_cb(&reload_rom);

        if (!sound_disabled) spu::init();
    }

    void update() {
        cpu::cycle();

        if (!cpu::stopped) {
            ppu::cycle();
            timer::update();
        }

        if (settings::enable_joyp_irq_delay) joypad::update();
    }
}