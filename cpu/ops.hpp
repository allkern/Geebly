#pragma once

#include "registers.hpp"
#include "../bus.hpp"

namespace gameboy {
    namespace cpu {
        #define Z 0b10000000
        #define N 0b01000000
        #define H 0b00100000
        #define C 0b00010000

        struct state {
            u8 opcode, imm8;
            u16 imm;
            bool jump = false;
            size_t pc_increment;
        } st;

        inline bool get_carry() {
            return registers::af & 0x10u;
        }

        inline void set_flags(u8 mask, bool cond, bool reset = true) {
            using namespace registers;
            if (cond) {
                r[f] |= mask;
                return;
            }
            if (reset) {
                r[f] &= ~mask;
            }
        }

        inline bool test_flags(u8 mask) {
            return registers::r[registers::f] & mask;
        }

        inline void push(u16 addr) {
            bus::write(registers::sp, addr, 2);
            registers::sp -= 2;
        }

        inline void pop(u16& dst) {
            dst = bus::read(registers::sp, 2);
            registers::sp += 2;
        }

        inline u16 pop() {
            u16 p = bus::read(registers::sp, 2);
            registers::sp += 2;
            return p;
        }

        inline void op_adc(u8& dst, u8 src, bool carry) {
            u16 res = dst;
            res += src + (int)carry;
            set_flags(Z, res==0);
            set_flags(N, false);
            set_flags(H, res&0x10);
            set_flags(C, res&0x100);
            dst = res;
        }

        inline void op_sbc(u8& dst, u8 src, bool carry) {
            u16 res = dst;
            res -= src - (int)carry;
            set_flags(Z, res==0);
            set_flags(N, true);
            set_flags(H, res&0x8);
            //set_flags(C, res&0x100);
        }

        inline void op_and(u8& dst, u8 src) {
            dst &= src;
            set_flags(Z, dst==0);
            set_flags(N, false);
            set_flags(H, true);
            set_flags(C, false);
        }

        inline void op_or(u8& dst, u8 src) {
            dst |= src;
            set_flags(Z, dst==0);
            set_flags(N, false);
            set_flags(H, false);
            set_flags(C, false);
        }

        inline void op_idc(u8& dst, bool dec) {
            if (dec) { dst--; } else { dst++; };
            set_flags(Z, dst==0);
            set_flags(N, dec);
            set_flags(H, false);
        }

        inline void op_xor(u8& dst, u8 src) {
            dst ^= src;
            set_flags(Z, dst==0);
            set_flags(N, false);
            set_flags(H, false);
            set_flags(C, false);
        }
    }
}