#ifdef __linux__
#define LOG_TARGET_LINUX
#endif

#ifdef _WIN32
#define LOG_TARGET_POWERSHELL
#endif

#include "gameboy.hpp"
#include "cpu/mnemonics.hpp"

#ifndef GEEBLY_NO_DEBUGGER
#include "debug.hpp"
#endif

#include "log.hpp"

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
    master_volume                                 = std::stod(cli::setting("master-volume", "1.0"));
    stereo                                        = !cli::setting("mono");
    sound_disabled                                = cli::setting("sound-disabled");
    
    _log::init("geebly");

    if (!settings::bios_checks_enabled) {
        bios::rom[0xfa] = 0x00;
        bios::rom[0xfb] = 0x00;
        bios::rom[0xe9] = 0x00;
        bios::rom[0xea] = 0x00;
    }

#ifndef GEEBLY_NO_DEBUGGER
    if (settings::debugger_enabled) debug::init();
#endif

    gameboy::init();

    while (screen::is_open()) {
        gameboy::update();

#ifndef GEEBLY_NO_DEBUGGER
        if (settings::debugger_enabled && debug::is_open()) debug::update();
#endif
    }
#ifndef GEEBLY_NO_DEBUGGER
    if (settings::debugger_enabled && debug::is_open()) debug::close();
#endif

    screen::close();

    cart::create_sav_file();

    return 0;
}
