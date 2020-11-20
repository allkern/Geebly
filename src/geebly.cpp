#include "aliases.hpp"
#include "cpu/thread.hpp"
#include "cpu/cpu.hpp"
#include "cpu/mnemonics.hpp"
#include "log.hpp"

#include "debug.hpp"
#include "cli.hpp"

#include <csignal>

using namespace gameboy;

void sigsegv_handler(int sig) {
    #ifdef __linux__
        char buf[256];

        sprintf(
            &buf[0],
            "zenity --error --width 300 --text \"SIGSEGV (%i) raised.\npc=%04x,\nsp=%04x,\naf=%04x,\nbc=%04x,\nde=%04x,\nhl=%04x,\ni=\'%s\' (%02x),\nimm=%04x,\nimm8=%02x,\nj=%i\"",
            sig,
            cpu::registers::pc,
            cpu::registers::sp,
            (u16)cpu::registers::af,
            (u16)cpu::registers::bc,
            (u16)cpu::registers::de,
            (u16)cpu::registers::hl,
            mnemonics[cpu::s.opcode].c_str(),
            cpu::s.opcode,
            cpu::s.imm,
            cpu::s.imm8,
            cpu::s.jump
        );

        int i = system(&buf[0]);
    #endif

    #ifndef __linux__
        _log(error,
            "SIGSEGV (%i) raised. pc=%04x, sp=%04x, af=%04x, bc=%04x, de=%04x, hl=%04x, i=%s (%02x), imm=%04x, imm8=%02x, j=%i",
            sig,
            cpu::registers::pc,
            cpu::registers::sp,
            (u16)cpu::registers::af,
            (u16)cpu::registers::bc,
            (u16)cpu::registers::de,
            (u16)cpu::registers::hl,
            mnemonics[cpu::s.opcode].c_str(),
            cpu::s.opcode,
            cpu::s.imm,
            cpu::s.imm8,
            cpu::s.jump
        );
    #endif
    
    std::exit(1);
}

int main(int argc, const char* argv[]) {
    cli::init(argc, argv);
    cli::parse();

    // This should probably be either automatic, or somewhere else
    debugger_enabled = cli::setting("debug");
    inaccessible_vram_emulation_enabled = !cli::setting("no-vram-access-emulation");
    bios_checks_enabled = !cli::setting("no-bios-checks");
    skip_bootrom = cli::setting("bootrom-skip");

    _log::log::init("geebly");

    // Clean this up
    std::signal(SIGSEGV, sigsegv_handler);

    bios::init(cli::setting("bios", "bios.bin"));

    // Patch infinite loops for 2 NOPs
    if (!bios_checks_enabled) {
        bios::rom[0xfa] = 0x00;
        bios::rom[0xfb] = 0x00;
        bios::rom[0xe9] = 0x00;
        bios::rom[0xea] = 0x00;
    }

    cart::insert_cartridge(cli::setting("cartridge", "undefined"));

    ppu::init(std::stoi(cli::setting("scale", "1")), cpu::registers::last_instruction_cycles);

    cpu::init();

    bus::init();

    if (debugger_enabled) {
        debug::init();
        init();
    }

    while (!window_closed) {
        if (!debugger_enabled) {
            cpu::fetch();
            cpu::execute();
        }

        if (debugger_enabled) {
            if (cpu::done) {
                ppu::cycle();
                cpu::done = false;
            }
        } else {
            ppu::cycle();
        }

        if (debugger_enabled) debug::update();
    }
}
