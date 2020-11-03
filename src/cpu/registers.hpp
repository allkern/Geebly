#pragma once

#include "../aliases.hpp"

#include <array>

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
        } s;

        std::atomic <bool> step;
        std::atomic <bool> run;

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

                pair& operator++(int) { (*low)++; if (!(*low)) { (*high)++; } return *this; }
                pair& operator--(int) { (*low)--; if (*low == 0xff) { (*high)--; } return *this; }

                pair(u8& high, u8& low) : high(&high), low(&low) {};
            };

            enum names { b, c, d, e, h, l, f, a };
        
            std::array <uint8_t, 8> r = { 0 };

            uint64_t cycles = 0;
            uint8_t last_instruction_cycles = 0;

            // Program Counter
            uint16_t pc = 0x100;

            // Stack Pointer
            uint16_t sp = 0xfffe;

            // Interrupt Master Enable
            bool ime = true;

            pair af { r[a], r[f] },
                 bc { r[b], r[c] },
                 de { r[d], r[e] },
                 hl { r[h], r[l] };
        }

        bool jump = false;
    }

}