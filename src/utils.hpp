#pragma once

#include "aliases.hpp"

namespace gameboy {
    namespace utils {
        template <typename T>
        bool bit(unsigned int n, T num) {
            return (num >> n) & 1;
        }

        template <typename T>
        T bits(unsigned int n, unsigned int m, T num) {
            T one_over = (m - n + 1) << 1;
            T mask = one_over - 1;
            return (num >> n) & mask;
        }

        template <typename T>
        u8 low3(T num) {
            return bits(0, 2, num);
        }

        template <typename T>
        u8 mid3(T num) {
            return bits(3, 5, num);
        }

        template <typename T>
        u8 low4(T num) {
            return bits(0, 3, num);
        }

        template <typename T>
        u8 low8(T num) {
            return bits(0, 7, num);
        }

        template <typename T>
        u16 low16(T num) {
            return bits(0, 15, num);
        }
    }
}