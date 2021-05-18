#ifdef __linux__
#define LOG_TARGET_LINUX 1
#else
#define LOG_TARGET_LINUX 0
#endif

#ifdef _WIN32
#define LOG_TARGET_POWERSHELL 1
#else
#define LOG_TARGET_POWERSHELL 0
#endif

#include "gameboy.hpp"
#include "cpu/mnemonics.hpp"
#include "debug/debug.hpp"

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

    gameboy::init();

    if (settings::debugger_enabled) {
        pause = true;
        step = false;

        debug::init();
    }

    while (screen::is_open()) {
        gameboy::update();

        if (settings::debugger_enabled && debug::is_open()) debug::update();
    }

    if (settings::debugger_enabled && debug::is_open()) debug::close();

    screen::close();

    cart::create_sav_file();

    return 0;
}
