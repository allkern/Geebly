#include "devices/ppu.hpp"
#include "thread.hpp"
#include "log.hpp"

#include "debug.hpp"

#include <csignal>

using namespace gameboy;

void sigsegv_handler(int sig) {
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
    std::exit(1);
}

int main() {
    _log::log::init("geebly");

    std::signal(SIGSEGV, sigsegv_handler);
    
    bios::init("bios.bin");

    cart::insert_cartridge("oprr.gb"); 

    debug::init();

    ppu::init(cpu::registers::last_instruction_cycles);

    init();

    while (true) {
        ppu::cycle();
        debug::update();
    }
}