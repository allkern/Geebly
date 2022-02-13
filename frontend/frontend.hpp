#pragma once

#include "audio.hpp"
#include "input.hpp"
#include "debug/debug.hpp"
#include "window.hpp"
#include "geebly/gameboy.hpp"

using namespace gameboy;

namespace frontend {
    void load_settings() {
        settings::bios_checks_enabled                 = !cli::setting("no-bios-checks");
        settings::enable_joyp_irq_delay               = !cli::setting("no-joyp-irq-delay");
        settings::inaccessible_vram_emulation_enabled = cli::setting("vram-access-emulation");
        settings::debugger_enabled                    = cli::setting("debug");
        settings::skip_bootrom                        = cli::setting("bootrom-skip");
        settings::cgb_mode                            = cli::setting("cgb-mode");
        settings::disable_logs                        = cli::setting("no-logs");
        settings::sgb_mode                            = cli::setting("sgb-mode");
        gameboy::master_volume                        = std::stod(cli::setting("master-volume", "1.0"));
        stereo                                        = !cli::setting("mono");
        gameboy::sound_disabled                       = cli::setting("sound-disabled");
        ntsc_codec_enabled                            = cli::setting("ntsc-codec");
        start_with_gui                                = cli::setting("gui") || cli::no_arguments();
        settings::disable_logs                        = start_with_gui;
        blend_frames                                  = cli::setting("blend-frames");
    }

    void rom_drop_cb(const char* file) {
        _log(debug, "%s", file);
    }

    void init() {
        ppu::init(settings::debugger_enabled ? nullptr : window::update);


        if (!settings::debugger_enabled) {
            window::register_keydown_cb(input::keydown_cb);
            window::register_keyup_cb(input::keyup_cb);
            window::register_rom_dropped_cb(gameboy::reload_rom);
            window::init(std::stoi(cli::setting("scale", "1")));
        }

        audio::init();
    }

    void close() {
        if (!settings::debugger_enabled)
            window::close();
    }
}