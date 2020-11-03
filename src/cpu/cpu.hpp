#pragma once

#include <iostream>
#include <cstdint>
#include <utility>
#include <memory>
#include <vector>
#include <atomic>
#include <array>

#include "../aliases.hpp"
#include "../bus.hpp"
#include "../log.hpp"

#include "registers.hpp"
#include "ops.hpp"

namespace gameboy {
    namespace cpu {
        inline void update(size_t pci, size_t ci, bool j = false) {
            registers::cycles += ci;
            registers::last_instruction_cycles = ci;
            s.pc_increment = pci;
            jump = j;
        }

        void fetch() {
            s.opcode = bus::read(registers::pc, 1);
            s.imm = bus::read(registers::pc+1, 2);
            s.imm8 = s.imm & 0xff;
        }

        bool execute() {
            using namespace registers;

            u8 opcode = s.opcode;

            if (run == false) { while (step) { usleep(1); } }
            
            switch (opcode) {
                // nop
                case 0x00: { update(1, 4); } break;

                // inc %r8;
                case 0x04: case 0x14: case 0x24: case 0x0c: case 0x1c: case 0x2c: case 0x3c:
                case 0x05: case 0x15: case 0x25: case 0x0d: case 0x1d: case 0x2d: case 0x3d: {
                    op_idc(r[(opcode >> 3) & 0x7], opcode & 1);
                    update(1, 4);
                } break;

                // inc *%hl, dec *%hl;
                case 0x34: case 0x35: {
                    op_idc(bus::ref(hl), opcode & 1);
                    update(1, 12);
                } break;

                // ld r8, r8
                case 0x40: case 0x41: case 0x42: case 0x43: case 0x44: case 0x45: case 0x47:
                case 0x48: case 0x49: case 0x4a: case 0x4b: case 0x4c: case 0x4d: case 0x4f:
                case 0x50: case 0x51: case 0x52: case 0x53: case 0x54: case 0x55: case 0x57:
                case 0x58: case 0x59: case 0x5a: case 0x5b: case 0x5c: case 0x5d: case 0x5f:
                case 0x60: case 0x61: case 0x62: case 0x63: case 0x64: case 0x65: case 0x67:
                case 0x68: case 0x69: case 0x6a: case 0x6b: case 0x6c: case 0x6d: case 0x6f:
                case 0x78: case 0x79: case 0x7a: case 0x7b: case 0x7c: case 0x7d: case 0x7f: {
                    r[(opcode >> 3) & 7] = r[opcode & 7];
                    update(1, 4);
                    break;
                } break;

                // ld r8, (hl)
                case 0x46: case 0x4e: case 0x56: case 0x5e: case 0x66: case 0x6e: case 0x76: case 0x7e: {
                    r[(opcode >> 3) & 7] = bus::read(hl, 1);
                    update(1, 8);
                    break;
                } break;

                // ld (hl), r8
                case 0x70: case 0x71: case 0x72: case 0x73: case 0x74: case 0x75: case 0x77: {
                    bus::write(hl, r[opcode & 0x7], 1);
                    update(1, 8);
                } break;

                // add a, r8; adc a, r8
                case 0x80: case 0x81: case 0x82: case 0x83: case 0x84: case 0x85: case 0x87:   
                case 0x88: case 0x89: case 0x8a: case 0x8b: case 0x8c: case 0x8d: case 0x8f: {
                    bool carry = (opcode & 8) && get_carry();
                    op_adc(r[a], r[opcode & 7], carry);
                    update(1, 4);
                } break;

                // add a, (hl); adc a, (hl)
                case 0x86: case 0x8e: {
                    bool carry = (opcode & 8) && get_carry();
                    op_adc(r[a], bus::read(hl, 1), carry);
                    update(1, 8);
                } break;

                // sub a, r8; sbc a, r8
                case 0x90: case 0x91: case 0x92: case 0x93: case 0x94: case 0x95: case 0x97:   
                case 0x98: case 0x99: case 0x9a: case 0x9b: case 0x9c: case 0x9d: case 0x9f: {
                    bool carry = (opcode & 8) && get_carry();
                    op_sbc(r[a], r[opcode & 7], carry);
                    update(1, 4);
                } break;

                case 0x96: case 0x9e: {
                    bool carry = (opcode & 8) && get_carry();
                    op_sbc(r[a], bus::read(hl, 1), carry);
                    update(1, 8);
                } break;

                // and a, r8; and a, (hl)
                case 0xa0: case 0xa1: case 0xa2: case 0xa3: case 0xa4: case 0xa5: case 0xa7: {
                    op_and(r[a], r[opcode & 7]);
                    update(1, 4);
                } break;

                case 0xa6: { op_and(r[a], bus::read(hl, 1)); update(1, 8); } break;

                // or a, r8; or a, (hl)
                case 0xb0: case 0xb1: case 0xb2: case 0xb3: case 0xb4: case 0xb5: case 0xb7: {
                    op_or(r[a], r[opcode & 7]);
                    update(1, 4);
                } break;

                case 0xb6: { op_or(r[a], bus::read(hl, 1)); update(1, 8); } break;

                // xor a, r8; xor a, (hl)
                case 0xa8: case 0xa9: case 0xaa: case 0xab: case 0xac: case 0xad: case 0xaf: {
                    op_xor(r[a], r[opcode & 7]);
                    update(1, 4);
                } break;

                case 0xae: { op_xor(r[a], bus::read(hl, 1)); update(1, 8); } break;

                // cp a, r8; cp a, (hl)
                case 0xb8: case 0xb9: case 0xba: case 0xbb: case 0xbc: case 0xbd: case 0xbf: {
                    op_cp(r[a], r[opcode & 7]);
                    update(1, 4);
                } break;

                case 0xbe: { op_cp(r[a], bus::read(hl, 1)); update(1, 8); } break;

                case 0xc6: case 0xce: {
                    bool carry = (opcode & 8) && get_carry();
                    op_adc(r[a], s.imm8, carry);
                    update(2, 8);
                } break;

                case 0xd6: case 0xde: {
                    bool carry = (opcode & 8) && get_carry();
                    op_sbc(r[a], s.imm8, carry);
                    update(2, 8);
                } break;

                case 0xe6: { op_and(r[a], s.imm8); update(2, 8); } break;
                case 0xee: { op_xor(r[a], s.imm8); update(2, 8); } break;
                case 0xf6: { op_or(r[a], s.imm8); update(2, 8); } break;
                case 0xfe: { op_cp(r[a], s.imm8); update(2, 8); } break;

                // ld r8, d8; ld *%hl, d8;
                case 0x06: case 0x16: case 0x26: case 0x0e: case 0x1e: case 0x2e: case 0x3e: {
                    r[(opcode & 0x38) >> 3] = s.imm8;
                    update(2, 8);
                } break;
                case 0x36: { bus::write(hl, s.imm8, 1); update(2, 12); } break;

                // rst #rst_vector;
                case 0xc7: case 0xd7: case 0xe7: case 0xf7: case 0xcf: case 0xdf: case 0xef: case 0xff: {
                    push(pc);
                    pc = opcode & 0x38;
                    update(1, 16, true);
                } break;

                // ld %r16, #imm16;
                case 0x01: { bc = s.imm; update(3, 12); } break;
                case 0x11: { de = s.imm; update(3, 12); } break;
                case 0x21: { hl = s.imm; update(3, 12); } break;
                case 0x31: { sp = s.imm; update(3, 12); } break;

                case 0x08: { bus::write(s.imm, sp, 2); update(3, 20); } break;
                
                // inc %r16;
                case 0x03: { bc++; update(1, 8); } break;
                case 0x13: { de++; update(1, 8); } break;
                case 0x23: { hl++; update(1, 8); } break;
                case 0x33: { sp++; update(1, 8); } break;

                // dec %r16;
                case 0x0b: { bc--; update(1, 8); } break;
                case 0x1b: { de--; update(1, 8); } break;
                case 0x2b: { hl--; update(1, 8); } break;
                case 0x3b: { sp--; update(1, 8); } break;

                // ld *%r16, %a;
                case 0x02: { bus::write(bc, r[a], 1); update(1, 8); } break;
                case 0x12: { bus::write(de, r[a], 1); update(1, 8); } break;

                // ld *%hl+-, %a;
                case 0x22: { bus::write(hl++, r[a], 1); update(1, 8); } break;
                case 0x32: { bus::write(hl--, r[a], 1); update(1, 8); } break;

                // ld *%r16, %a;
                case 0x0a: { r[a] = bus::read(bc, 1); update(1, 8); } break;
                case 0x1a: { r[a] = bus::read(de, 1); update(1, 8); } break;

                // ld %a, *%hl+-;
                case 0x2a: { r[a] = bus::read(hl++, 1); update(1, 8); } break;
                case 0x3a: { r[a] = bus::read(hl--, 1); update(1, 8); } break;

                // TO-DO: implement 16-bit carry and half carry detection
                // add %hl, %r16;
                case 0x09: { hl = hl + bc; set_flags(N, false); update(1, 8); } break;
                case 0x19: { hl = hl + de; set_flags(N, false); update(1, 8); } break;
                case 0x29: { hl = hl + hl; set_flags(N, false); update(1, 8); } break;
                case 0x39: { hl = hl + sp; set_flags(N, false); update(1, 8); } break;

                case 0x20: { if (!test_flags(Z)) { pc += (s8)s.imm8 + 2; update(2, 12, true); } else { update(2, 8); } } break;
                case 0x30: { if (!test_flags(C)) { pc += (s8)s.imm8 + 2; update(2, 12, true); } else { update(2, 8); } } break;
                case 0x18: { pc += (s8)s.imm8 + 2; update(2, 12, true); } break;
                case 0x28: { if ( test_flags(Z)) { pc += (s8)s.imm8 + 2; update(2, 12, true); } else { update(2, 8); } } break;
                case 0x38: { if ( test_flags(C)) { pc += (s8)s.imm8 + 2; update(2, 12, true); } else { update(2, 8); } } break;
                
                case 0xc1: { bc = pop(); update(1, 12); } break;
                case 0xd1: { de = pop(); update(1, 12); } break;
                case 0xe1: { hl = pop(); update(1, 12); } break;
                case 0xf1: { af = pop(); update(1, 12); } break;
                
                case 0xc5: { push(bc); update(1, 12); } break;
                case 0xd5: { push(de); update(1, 12); } break;
                case 0xe5: { push(hl); update(1, 12); } break;
                case 0xf5: { push(af); update(1, 12); } break;

                case 0xc0: { if (!test_flags(Z)) { pc = pop(); update(1, 20, true); } else { update(1, 8); } } break;
                case 0xd0: { if (!test_flags(C)) { pc = pop(); update(1, 20, true); } else { update(1, 8); } } break;
                case 0xc8: { if ( test_flags(Z)) { pc = pop(); update(1, 20, true); } else { update(1, 8); } } break;
                case 0xd8: { if ( test_flags(C)) { pc = pop(); update(1, 20, true); } else { update(1, 8); } } break;
                case 0xc9: { pc = pop(); update(1, 16); } break;
                
                case 0xc2: { if (!test_flags(Z)) { pc = s.imm; update(3, 16, true); } else { update(3, 12); } } break;
                case 0xd2: { if (!test_flags(C)) { pc = s.imm; update(3, 16, true); } else { update(3, 12); } } break;
                case 0xca: { if ( test_flags(Z)) { pc = s.imm; update(3, 16, true); } else { update(3, 12); } } break;
                case 0xda: { if ( test_flags(C)) { pc = s.imm; update(3, 16, true); } else { update(3, 12); } } break;
                case 0xc3: { pc = s.imm; update(3, 16, true); } break;

                case 0xf8: { hl = sp + (s8)s.imm8; update(2, 12); } break;
                case 0xf9: { sp = (u16)hl; update(1, 8); } break;

                // ld *#i16, %a;
                case 0xea: { bus::write(s.imm, r[a], 1); update(3, 16); } break;

                // ld %a, *#i16;
                case 0xfa: { r[a] = bus::read(s.imm, 1); update(3, 16); } break;

                // add %sp, #si8;
                // TO-DO implement carry and halfcarry detection
                case 0xe8: { sp += (s8)s.imm8; set_flags(Z | N, false); update(2, 16); } break;

                // jp %hl;
                case 0xe9: { pc = (u16)hl; set_flags(1, 4, true); } break;

                // call[z,c,nz,nc] #i16;
                case 0xc4: { if (!test_flags(Z)) { push(pc); pc = s.imm; update(3, 24, true); } else { update(3, 12); } } break;
                case 0xd4: { if (!test_flags(C)) { push(pc); pc = s.imm; update(3, 24, true); } else { update(3, 12); } } break;
                case 0xcc: { if ( test_flags(Z)) { push(pc); pc = s.imm; update(3, 24, true); } else { update(3, 12); } } break;
                case 0xdc: { if ( test_flags(C)) { push(pc); pc = s.imm; update(3, 24, true); } else { update(3, 12); } } break;
                
                // call #i16;
                case 0xcd: { push(pc); pc = s.imm; update(3, 24, true); } break;

                // ld *#0xff00+#i8, %a; ld %a, *#0xff00+#i8;
                case 0xe0: { bus::write(0xff00 + s.imm8, r[a], 1); update(2, 12); } break;
                case 0xf0: { r[a] = bus::read(0xff00 + s.imm8, 1); update(2, 12); } break;

                // ld *#0xff00+%c, a, ld a, *#0xff00+%c;
                case 0xe2: { bus::write(0xff00 + r[c], r[a], 1); update(1, 8); } break;
                case 0xf2: { r[a] = bus::read(0xff00 + r[c], 1); update(1, 8); } break;

                // ei; di;
                case 0xf3: { ime = false; update(1, 4); } break;
                case 0xfb: { ime = true; update(1, 4); } break;

                // reti
                case 0xd9: { pc = pop(); update(1, 16); ime = true; } break;

                default: {
                    _log(error, "Unimplemented opcode 0x%02x @ pc=%04x", opcode, pc);
                    s.pc_increment = 0; // Softlock execution
                    //return false; // Halt and Catch Fire!
                }
            }

            if (!jump) { pc += s.pc_increment; }
            jump = false;
            step = true;

            return true;
        }
        
    };
}