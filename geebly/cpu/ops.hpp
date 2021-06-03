#pragma once

#include "registers.hpp"
#include "../bus.hpp"

namespace gameboy {
    namespace cpu {
        inline bool get_carry() {
            return registers::af.operator&(0x10u);
        }

        inline static void set_flags(u8 mask, bool cond, bool reset = true) {
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
            registers::sp -= 2;
            bus::write(registers::sp, addr, 2);
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

        void call(u16 addr, int off = 0) {
            push(registers::pc+off);
            registers::pc = addr;
            jump = true;
        }

        inline void op_rlc(u8& dst, bool thru, bool cb = false) {
            int old_c = test_flags(CF),
                msb = dst & 0x80;
            set_flags(CF, msb);
            dst <<= 1;
            if (thru) {
                dst |= old_c;
            } else {
                dst |= msb ? 1 : 0;
            }
            set_flags(ZF, cb ? dst == 0 : false);
            set_flags(NF | HF, false);
        }

        inline void op_sla(u8& dst) {
            set_flags(CF, dst&0x80);
            dst <<= 1;
            set_flags(ZF, dst==0);
            set_flags(NF | HF, false);
        }

        inline void op_sra(u8& dst, bool l) { // 1754
            int msb = dst & 0x80;
            set_flags(CF, dst&0x1);
            dst >>= 1;
            if (!l) { dst |= msb; };
            set_flags(ZF, dst==0);
            set_flags(NF | HF, false);
        }

        inline void op_swap(u8& dst) {
            dst = ((dst & 0xf) << 4 | (dst & 0xf0) >> 4);
            set_flags(ZF, dst==0);
            set_flags(NF | HF | CF, false);
        }

        inline void op_rrc(u8& dst, bool thru, bool cb = false) {
            int old_c = test_flags(CF) << 7,
                lsb = (dst & 0x1) << 7;
            set_flags(CF, lsb);
            dst >>= 1;
            if (thru) {
                dst |= old_c;
            } else {
                dst |= lsb;
            }
            set_flags(ZF, cb ? dst == 0 : false);
            set_flags(NF | HF, false);
        }

        inline void op_daa(u8& dst) {
            if (!test_flags(NF)) {
                if (test_flags(CF) || dst > 0x99) { dst += 0x60; set_flags(CF, true); }
                if (test_flags(HF) || (dst & 0x0f) > 0x09) { dst += 0x6; }
            } else {
                if (test_flags(CF)) { dst -= 0x60; }
                if (test_flags(HF)) { dst -= 0x6; }
            }
            set_flags(ZF, dst==0);
            set_flags(HF, false);
        }

        inline void op_adc(u8& dst, u8 src, bool carry) {
            u16 res = dst, hct = (dst & 0xf) + (src & 0xf) + (int)carry;
            res += src;
            res += (int)carry;
            set_flags(ZF, (u8)res==0);
            set_flags(NF, false);
            set_flags(HF, hct&0x10);
            set_flags(CF, res&0x100);
            dst = res;
        }

        inline void op_add(u16& dst, u8 src) {
            u32 res = dst;
            res += src;
        }

        inline void op_sbc(u8& dst, u8 src, bool carry) {
            u16 res = 0;
            res = dst - src - (int)carry;
            set_flags(ZF, (res & 0xff) == 0);
            set_flags(NF, true);
            set_flags(HF, (u8)((dst & 0xf) - (src & 0xf) - carry) > 0xf);
            set_flags(CF, (u16)(dst - src - carry) > 0xff);
            dst = res & 0xff;
        }

        inline void op_rst(u8& dst, int b, bool set) {
            if (set) {
                dst |= (1 << b);
            } else {
                dst &= ~(1 << b);
            }
        }

        inline void op_bit(u8& src, int b) {
            set_flags(ZF, !(src&(1<<b)));
            set_flags(NF, false);
            set_flags(HF, true);
        }

        inline void op_and(u8& dst, u8 src) {
            dst &= src;
            set_flags(ZF, dst==0);
            set_flags(NF | CF, false);
            set_flags(HF, true);
        }

        inline void op_or(u8& dst, u8 src) {
            dst |= src;
            set_flags(ZF, dst==0);
            set_flags(NF | HF | CF, false);
        }

        inline void op_idc(u8& dst, bool dec) {
            set_flags(HF, (dst & 0xf) == (dec ? 0 : 0xf));
            if (dec) { dst--; } else { dst++; };
            set_flags(ZF, !dst);
            set_flags(NF, dec);
        }

        inline void op_addhl(u16 src) {
            u32 res = (u16)registers::hl + src;
            set_flags(NF, false);
            set_flags(CF, res>0xffff);
            set_flags(HF, (((u16)registers::hl & 0xfff) + (src&0xfff)) > 0xfff);
            registers::hl = res & 0xffff;
        }

        inline void op_xor(u8& dst, u8 src) {
            dst ^= src;
            set_flags(ZF, dst==0);
            set_flags(NF | HF | CF, false);
        }

        inline void op_cp(u8& dst, u8 src) {
            u32 temp = dst - src; int hct = (dst & 0xf) - (src & 0xf);
            set_flags(ZF, (temp & 0xff) == 0);
            set_flags(NF, true);
            set_flags(HF, hct < 0);
            set_flags(CF, dst < src);
        }
    }
}