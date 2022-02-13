#pragma once

#include "../aliases.hpp"

#include <array>

namespace gameboy {
    namespace cpu {
        #define ZF 0b10000000
        #define NF 0b01000000
        #define HF 0b00100000
        #define CF 0b00010000

        struct state {
            u8 opcode = 0x0, imm8 = 0x0;
            u16 imm = 0x0;
            bool jump = false;
            size_t pc_increment = 0x0;
        } s;

        bool halted;//, stopped;

        namespace registers {
            struct pair {
                u8 *high, *low;

                #define BINARY_OP(b) inline u16 operator b(u16 value) const { return this->operator u16() b value; }

                pair& operator=(u16 nv) { *high = (nv & 0xff00) >> 8; *low = (nv & 0xff); return *this; }
                operator u16() const { return ((u16)*high << 8) | *low; }

                BINARY_OP(&)
                BINARY_OP(|)
                BINARY_OP(^)
                BINARY_OP(+)
                BINARY_OP(-)
                BINARY_OP(*)
                BINARY_OP(/)

                u16 operator++(int) { u16 old = (u16)*this; (*low)++; if (!(*low)) { (*high)++; } return old; }
                u16 operator--(int) { u16 old = (u16)*this; (*low)--; if (*low == 0xff) { (*high)--; } return old; }
                pair& operator++() { (*low)++; if (!(*low)) { (*high)++; } return *this; }
                pair& operator--() { (*low)--; if (*low == 0xff) { (*high)--; } return *this; }

                pair(u8& high, u8& low) : high(&high), low(&low) {};
            };

            enum names { b, c, d, e, h, l, f, a };
        
            std::array <uint8_t, 8> r = { 0 };

            uint64_t cycles = 0;
            int last_instruction_cycles = 0;

            // Program Counter
            uint16_t pc = 0x0;

            // Stack Pointer
            uint16_t sp = 0x0;

            // Interrupt Master Enable
            bool ime = false;

            pair af { r[a], r[f] },
                 bc { r[b], r[c] },
                 de { r[d], r[e] },
                 hl { r[h], r[l] };
        }

        bool jump = false;

        bool halt_bug = false,
             halt_ime_state = false,
             ei_issued = false;
        int  ei_delay = 0;

        u8 fired = 0;
    }

}