#pragma once

#include "aliases.hpp"
#include "global.hpp"

#include "cpu/cpu.hpp"
#include "cpu/mnemonics.hpp"
#include "log.hpp"
#include "cli.hpp"

namespace gameboy {
    bool file_exists(std::string name) {
        std::ifstream f(name);

        return f.good() && f.is_open();
    }

    double prev_volume;

    void mute() {
        prev_volume = master_volume;
        master_volume = 0.0;
    }

    void unmute() {
        master_volume = prev_volume;
    }

    void load_bootrom();

    // To-do: Make ROM dropping better
    void reset() {
        if (!sound_disabled)
            spu::reset();
        
        if (!settings::skip_bootrom)
            load_bootrom();

        wram::init();
        cpu::init();
        bus::init();
        hram::init();
        clock::reset();
        ic::reset();
        joypad::reset();
        timer::reset();
        dma::reset();
        hdma::reset();
        ppu::reset_full();
    }

    void save_compatibility_checks(std::ofstream& o) {
        // Save the ROM name and checksum as checks
        std::string file = cart::get_filename();
        int size = file.size();

        GEEBLY_WRITE_VARIABLE(size);

        o.write(file.c_str(), file.size());

        GEEBLY_WRITE_VARIABLE(cart::global_checksum);
    }

    std::string state_rom;

    bool load_and_check_compatibility(std::ifstream& i) {
        int rom_name_size = 0;
        uint16_t save_global_checksum = 0;

        GEEBLY_LOAD_VARIABLE(rom_name_size);

        state_rom.resize(rom_name_size);

        i.read((char*)state_rom.data(), rom_name_size);

        GEEBLY_LOAD_VARIABLE(save_global_checksum);

        return (save_global_checksum == cart::global_checksum) &&
               (state_rom == cart::get_filename());
    }

    void save_state(std::string name) {
        std::ofstream ss(name, std::ios::binary);

        save_compatibility_checks(ss);

        wram::save_state(ss);
        cpu::save_state(ss);
        bus::save_state(ss);
        hram::save_state(ss);
        clock::save_state(ss);
        ic::save_state(ss);
        joypad::save_state(ss);
        timer::save_state(ss);
        dma::save_state(ss);
        hdma::save_state(ss);
        ppu::save_state(ss);
        cart::save_state(ss);
    }

    bool compatible = true;

    bool load_state(std::string name, bool load_anyways = false) {
        std::ifstream ss(name, std::ios::binary);

        compatible = load_and_check_compatibility(ss);

        if ((!compatible) && (!load_anyways)) return false;

        reset();

        wram::load_state(ss);
        cpu::load_state(ss);
        bus::load_state(ss);
        hram::load_state(ss);
        clock::load_state(ss);
        ic::load_state(ss);
        joypad::load_state(ss);
        timer::load_state(ss);
        dma::load_state(ss);
        hdma::load_state(ss);
        ppu::load_state(ss);
        cart::load_state(ss);

        return true;
    }

    void reload_rom(const char* file) {
        reset();

        cart::insert_cartridge(file);

        cpu::handle_interrupts();
        cpu::fetch();

        pause = settings::debugger_enabled;
        step = true;
    }

    void load_bootrom() {
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

        settings::skip_bootrom = settings::skip_bootrom || !bootrom_present;

        if (!settings::skip_bootrom)
            boot::init(cli::setting("boot", settings::cgb_mode ? "cgb_boot.bin" : "dmg_boot.bin"));
    }

    void init() {
        if (settings::disable_logs) _log::disable();

        load_bootrom();

        if (!cli::is_defined("cartridge"))
            _log(warning, "No cartridge inserted, loading no_cart");

        cart::insert_cartridge(cli::setting("cartridge", "geebly-no-cart"));
        reset();
        clock::init(cpu::registers::last_instruction_cycles);

        // We reverse the order of execution to enable
        // the debugger to observe values before an instruction
        // has been executed
        if (settings::debugger_enabled)
            cpu::fetch();

        pause = settings::debugger_enabled;
        step = false;

        if (!settings::bios_checks_enabled) {
            boot::rom[0xfa] = 0x00;
            boot::rom[0xfb] = 0x00;
            boot::rom[0xe9] = 0x00;
            boot::rom[0xea] = 0x00;
        }

        if (!sound_disabled) spu::init();
    }

    void update() {
        if (!pause) {
            cpu::cycle();

            //if (cpu::registers::pc == 0x169) { cpu::fetch(); step = false; pause = true; master_volume = 1.0 * (!pause); }

            ppu::cycle();
            timer::update();
            spu::update();

            if (settings::enable_joyp_irq_delay)
                joypad::update();
        } else {
            if (step) {
                cpu::execute();
                cpu::handle_interrupts();
                cpu::fetch();
                spu::update();

                ppu::cycle();
                timer::update();

                if (settings::enable_joyp_irq_delay)
                    joypad::update();

                step = false;
            }
        }

        //if (cpu::registers::pc == 0x773) { step = false; pause = true; master_volume = 1.0 * (!pause); }
        //if (cpu::registers::pc == 0xc2c9) { step = false; pause = true; master_volume = 1.0 * (!pause); }
    }
}
// 002cdbf4 002cde8c
// 00b2d5e6 00b2d596