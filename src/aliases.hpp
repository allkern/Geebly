#pragma once

#include <cstdint>

namespace gameboy {
    typedef uint32_t u32;
    typedef uint16_t u16;
    typedef uint8_t  u8;
    typedef  int8_t  s8;
#ifndef _WIN32
    typedef std::size_t size_t;
#endif
}