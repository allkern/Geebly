#pragma once

#include "../../aliases.hpp"

#include "translate.hpp"

#include <cstring>

#define HDMA_BEGIN 0xff51
#define HDMA_END 0xff55

#define HDMA_SRCH 0xff51
#define HDMA_SRCL 0xff52
#define HDMA_DSTH 0xff53
#define HDMA_DSTL 0xff54
#define HDMA_CTRL 0xff55

namespace gameboy {
    namespace hdma {
        enum mode_t : bool {
            gdma = false,
            hdma = true
        };

        u8 dummy;

        u16 src = 0x0, dst = 0x0, len = 0x0;

        bool mode = mode_t::gdma;

        u32 read(u16 addr, size_t size) {
            return 0xff;
        }

        void write(u16 addr, u16 value, size_t size) {
            if (addr == HDMA_SRCH) src = (src & 0xff) | ((value & 0xff) << 8);
            if (addr == HDMA_SRCL) src = (src & 0xff00) | (value & 0xff);
            if (addr == HDMA_DSTH) dst = (dst & 0xff) | ((value & 0xff) << 8);
            if (addr == HDMA_DSTL) dst = (dst & 0xff00) | (value & 0xff);

            if (addr == HDMA_CTRL) {
                mode = value & 0x80;
                len = ((value & 0x7f) + 1) << 4;

                src &= 0xfff0;
                dst = 0x8000 + (dst & 0x1ff0);

                u8 *src_ptr = dma::translate(src),
                   *dst_ptr = dma::translate(dst);

                if (!src_ptr) _log(warning, "%s transfer failure, source invalid", mode ? "HDMA" : "GDMA");
                if (!dst_ptr) _log(warning, "%s transfer failure, destination invalid", mode ? "HDMA" : "GDMA");

                if (src_ptr && dst_ptr) std::memcpy(dst_ptr, src_ptr, len);
            }
        }

        u8& ref(u16 addr) {
            return dummy;
        }
    }
}