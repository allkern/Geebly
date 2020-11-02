//include "bus.hpp"
#include "cpu/cpu.hpp"
#include "log.hpp"

#include "debug.hpp"

using namespace gameboy;

int main() {
    _log::log::init("geebly");
    
    bios::init("bios.bin");

    cart::insert_cartridge("smb.gb");

    debug_window dw;

    dw.start();

    ppu::init(cpu::registers::last_instruction_cycles);

    bool exec = true;
    while (exec) {
        cpu::fetch();
        //_log(debug,
        //    "pc=%04x\topcode=%02x\timm=%04x\timm8=%02x",
        //    cpu::registers::pc,
        //    cpu::st.opcode,
        //    cpu::st.imm,
        //    cpu::st.imm8
        //);
        exec = cpu::execute();
        //_log(debug,
        //    "pc=%04x\taf=%04x\tbc=%04x\tde=%04x\thl=%04x\tsp=%04x\n",
        //    cpu::registers::pc,
        //    (u16)cpu::registers::af,
        //    (u16)cpu::registers::bc,
        //    (u16)cpu::registers::de,
        //    (u16)cpu::registers::hl,
        //    cpu::registers::sp
        //);
        ppu::cycle();
    }
}