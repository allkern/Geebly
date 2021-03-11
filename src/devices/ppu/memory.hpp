#pragma once

#include "../../aliases.hpp"

#define LGW_FORMAT_ARGB8888

#include "lgw/framebuffer.hpp"

#include <queue>
#include <array>

#define PPU_BEGIN   0x8000
#define PPU_END     0x9fff
#define VRAM_BEGIN  PPU_BEGIN
#define VRAM_END    PPU_END
#define CRAM_BEGIN  PPU_BEGIN
#define CRAM_END    0x97ff
#define TM0_BEGIN   0x9800
#define TM0_END     0x9bff
#define TM1_BEGIN   0x9c00
#define TM1_END     PPU_END
#define OAM_BEGIN   0xfe00
#define OAM_END     0xfe9f
#define PPU_R_BEGIN 0xff40
#define PPU_R_END   0xff4b

#define PPU_WIDTH  160
#define PPU_HEIGHT 144

namespace gameboy {
    namespace ppu {
                struct fifo_pixel_t {
            u8 color = 0,
               palette = 0,
               oam_index = 0;

            bool bg_priority = false;
        };

        enum mode {
            MODE_HBLANK,
            MODE_VBLANK,
            MODE_SPR_SEARCH,
            MODE_DRAW
        };

        typedef std::array <u8, 0x2000>     vram_bank_t;
        typedef std::array <u8, 0xa0>       oam_t;
        typedef std::array <u8, 0x40>       cgb_palette_ram_t;
        typedef std::array <u8, 0xc>        registers_t;
        typedef std::array <u32, 4>         dmg_palette_t;
        typedef std::array <vram_bank_t, 2> vram_t;
        typedef std::queue <fifo_pixel_t>   fifo_t;

        typedef lgw::framebuffer <PPU_WIDTH, PPU_HEIGHT> framebuffer_t;

        vram_t vram;
        oam_t oam;

        u8 cgb_bg_palette_idx = 0,
           cgb_spr_palette_idx = 0,
           current_bank_idx = 0,
           dummy = 0;

        registers_t r = { 0 };
        cgb_palette_ram_t cgb_bg_palette = { 0 };
        cgb_palette_ram_t cgb_spr_palette = { 0 };

        fifo_t background_fifo,
               sprite_fifo;

        vram_bank_t* current_bank = &vram[current_bank_idx];

        // framebuffer_t frame;

        dmg_palette_t dmg_palette = {
            0xfffffffful,
            0xffaaaaaaul,
            0xff555555ul,
            0xff000000ul
        };

        bool bg_auto_inc = false,
             spr_auto_inc = false;

        size_t clk = 0, cx, rx, sx, sy, fx;

        u8 l, h;

        // u32 read(u16 addr, size_t size) {
        //     if (addr >= PPU_R_BEGIN && addr <= PPU_R_END) {
        //         return utility::default_mb_read(r.data(), addr, size, PPU_R_BEGIN);
        //     }

        //     if (addr >= VRAM_BEGIN && addr <= VRAM_END) {
        //         return utility::default_mb_read(current_bank->data(), addr, size, VRAM_BEGIN);
        //     }

        //     if (addr >= OAM_BEGIN && addr <= OAM_END) {
        //         return utility::default_mb_read(oam.data(), addr, size, OAM_BEGIN);
        //     }

        //     return 0;
        // }

        // void write(u16 addr, u16 value, size_t size) {
        //     if (addr >= PPU_R_BEGIN && addr <= PPU_R_END) {
        //         utility::default_mb_write(r.data(), addr, value, size, PPU_R_BEGIN);
        //         return;
        //     }

        //     if (addr >= VRAM_BEGIN && addr <= VRAM_END) {
        //         utility::default_mb_write(current_bank->data(), addr, value, size, VRAM_BEGIN);
        //         return;
        //     }

        //     if (addr >= OAM_BEGIN && addr <= OAM_END) {
        //         utility::default_mb_write(oam.data(), addr, value, size, OAM_BEGIN);
        //         return;
        //     }
        // }

        // u8& ref(u16 addr) {
        //     if (addr >= PPU_R_BEGIN && addr <= PPU_R_END) { return r[addr-PPU_R_BEGIN]; }
            
        //     if (addr >= VRAM_BEGIN && addr <= VRAM_END) { return (*current_bank)[addr-VRAM_BEGIN]; }

        //     if (addr >= OAM_BEGIN && addr <= OAM_END) { return oam[addr-OAM_BEGIN]; }

        //     return dummy;
        // }
    }
}