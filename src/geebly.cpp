#include "devices/ppu.hpp"

//#define GEEBLY_DEBUG
#include "thread.hpp"
#include "log.hpp"

#include "debug.hpp"

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

int main() {
    _log::log::init("geebly");

    std::signal(SIGSEGV, sigsegv_handler);
    
    bios::init("bios.bin");

    cart::insert_cartridge("test.gb");

    #ifdef GEEBLY_DEBUG
    debug::init();
    #endif

    ppu::init(cpu::registers::last_instruction_cycles);

    //cpu::registers::pc = 0x38;

    init();

    sf::Clock perf;

    perf.restart();
    while (true) {
        if (cpu::done) {
            ppu::cycle();
            cpu::done = false;
        }

        #ifdef GEEBLY_DEBUG
        debug::update();
        #endif
    }
}