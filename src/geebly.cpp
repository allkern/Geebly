//#define GEEBLY_NO_SOUND

#include "aliases.hpp"
//#include "cpu/thread.hpp"
#include "cpu/cpu.hpp"
#include "cpu/mnemonics.hpp"
#include "log.hpp"

#include "global.hpp"
//#include "debug.hpp"
#include "cli.hpp"

using namespace gameboy;

int main(int argc, char *argv[]) {
    cli::init(argc, argv);
    cli::parse();

    // This should probably be either automatic, or somewhere else
    settings::bios_checks_enabled                 = !cli::setting("no-bios-checks");
    settings::enable_joyp_irq_delay               = !cli::setting("no-joyp-irq-delay");
    settings::inaccessible_vram_emulation_enabled = cli::setting("vram-access-emulation");
    settings::debugger_enabled                    = cli::setting("debug");
    settings::skip_bootrom                        = cli::setting("bootrom-skip");
    settings::cgb_mode                            = cli::setting("cgb-mode");
    
    _log::init("geebly");

    // Clean this up
    //std::signal(SIGSEGV, sigsegv_handler);

    if (!settings::skip_bootrom)
        bios::init(cli::setting("bios", settings::cgb_mode ? "cgb_bios.bin" : "bios.bin"));

    // Patch infinite loops for 2 NOPs
    if (!settings::bios_checks_enabled) {
        bios::rom[0xfa] = 0x00;
        bios::rom[0xfb] = 0x00;
        bios::rom[0xe9] = 0x00;
        bios::rom[0xea] = 0x00;
    }

    if (!cli::is_defined("cartridge")) _log(warning, "No cartridge inserted, loading no_cart");

    cart::insert_cartridge(cli::setting("cartridge", "geebly-no-cart"));

    wram::init();
    
    hram::init();
    
    ppu::init(std::stoi(cli::setting("scale", "1")));
    
    cpu::init();

    bus::init();

    clock::init(cpu::registers::last_instruction_cycles);

    //screen::init(&ppu::frame, std::stoi(cli::setting("scale", "1")));

#ifdef _WIN32
#ifndef GEEBLY_NO_SOUND
    spu::init();
#endif
#endif

    if (settings::debugger_enabled) {
        //debug::init();
        //cpu_thread::init();
    }

    int counter = 1000;

    while (!window_closed) {
        if (!settings::debugger_enabled) {
            cpu::cycle();
        }

        if (settings::debugger_enabled) {
            if (cpu::done) {
                if (!cpu::stopped) ppu::cycle();
                if (!cpu::stopped) timer::update();
                #ifdef _WIN32
                #ifndef GEEBLY_NO_SOUND
                    if (!cpu::stopped) spu::update();
                #endif
                #endif
                cpu::done = false;
            }
        } else {
            if (!cpu::stopped) ppu::cycle();
            if (!cpu::stopped) timer::update();
            #ifdef _WIN32
            #ifndef GEEBLY_NO_SOUND
                if (!cpu::stopped) spu::update();
            #endif
            #endif
            //if (settings::enable_joyp_irq_delay) joypad::update();
        }

        if (settings::debugger_enabled) {
            // if (debug::run) {
            //     if (!(counter--)) {
            //         debug::update();
            //         counter = 1000;
            //     }
            // } else {
            //     debug::update();
            // }
        }
    }

    ppu::close();

    cart::create_sav_file();

    return 0;
}
