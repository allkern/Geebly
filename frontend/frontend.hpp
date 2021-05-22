#pragma once

#include "audio.hpp"
#include "window.hpp"
#include "input.hpp"
#include "debug/debug.hpp"

using namespace gameboy;

namespace frontend {
    void load_settings() {
        settings::bios_checks_enabled                 = !cli::setting("no-bios-checks");
        settings::enable_joyp_irq_delay               = !cli::setting("no-joyp-irq-delay");
        settings::inaccessible_vram_emulation_enabled = cli::setting("vram-access-emulation");
        settings::debugger_enabled                    = cli::setting("debug");
        settings::skip_bootrom                        = cli::setting("bootrom-skip");
        settings::cgb_mode                            = cli::setting("cgb-mode");
        master_volume                                 = std::stod(cli::setting("master-volume", "1.0"));
        stereo                                        = !cli::setting("mono");
        sound_disabled                                = cli::setting("sound-disabled");
    }

    void init() {
        ppu::init(screen::update);

        screen::register_keydown_cb(input::keydown_cb);
        screen::register_keyup_cb(input::keyup_cb);
        screen::init(std::stoi(cli::setting("scale", "1")));

        audio::init();
    }

    void close() {
        screen::close();
    }
}