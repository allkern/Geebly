#pragma once

#include "../../aliases.hpp"

#ifdef GEEBLY_FRAMEBUFFER_FORMAT
#if GEEBLY_FRAMEBUFFER_FORMAT == 1
#define LGW_FORMAT_ARGB8888
#elif GEEBLY_FRAMEBUFFER_FORMAT == 2
#define LGW_FORMAT_RGBA8888
#endif
#else
#define LGW_FORMAT_ARGB8888
#endif

#include "lgw/framebuffer.hpp"

#include <algorithm>
#include <vector>
#include <queue>
#include <array>

#define MMIO_VBK    0xff4f
#define MMIO_BCPS   0xff68
#define MMIO_BCPD   0xff69
#define MMIO_OCPS   0xff6a
#define MMIO_OCPD   0xff6b

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
#define IOAM_BEGIN  0xfea0
#define IOAM_END    0xfeff
#define PPU_R_BEGIN 0xff40
#define PPU_R_END   0xff4b

#define PPU_WIDTH  160
#define PPU_HEIGHT 144

// PPU register indexes
#define PPU_LCDC    0x0
#define PPU_STAT    0x1
#define PPU_SCY     0x2
#define PPU_SCX     0x3
#define PPU_LY      0x4
#define PPU_LYC     0x5
#define PPU_BGP     0x7
#define PPU_OBP0    0x8
#define PPU_OBP1    0x9
#define PPU_WY      0xa
#define PPU_WX      0xb

// LCDC masks
#define LCDC_SWITCH 0b10000000
#define LCDC_WNDTMS 0b01000000
#define LCDC_WNDSWI 0b00100000
#define LCDC_BGWTSS 0b00010000
#define LCDC_BGWTMS 0b00001000
#define LCDC_SPSIZE 0b00000100
#define LCDC_SPDISP 0b00000010 
#define LCDC_BGWSWI 0b00000001

// STAT masks
#define STAT_LYCNSD 0b01000000
#define STAT_MODE02 0b00100000
#define STAT_MODE01 0b00010000
#define STAT_MODE00 0b00001000
#define STAT_CDFLAG 0b00000100
#define STAT_CRMODE 0b00000011

// Sprite attribute masks
#define SPATTR_PRIO 0b10000000
#define SPATTR_YFLP 0b01000000
#define SPATTR_XFLP 0b00100000
#define SPATTR_PALL 0b00010000

// CGB mode
#define SPATTR_TVBK 0b00001000
#define SPATTR_PALN 0b00000111

#define TEST_REG(reg, mask) (r[reg] & mask)

namespace gameboy {
    namespace ppu {        
        template <class T> class queue_t {
            std::vector <T> c;

        public:
            void push(T value) {
                c.push_back(value);
            }

            void pop() {
                if (c.size()) {
                    c.erase(c.begin());
                } else {
                    _log(debug, "TRYING TO POP FROM AN EMPTY QUEUE");
                }
            }

            T front() {
                return c.at(0);
            }

            size_t size() {
                return c.size();
            }

            bool empty() {
                return !size();
            }
        };

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

        struct sprite_t {
            u8 y : 8,
               x : 8,
               t : 8,
               a : 8;
        };

        struct cgb_bg_attribute {
            u8 bgp;
            bool vram_bank, xflip, yflip, priority;
        } bg_attr;

        typedef void (*frame_ready_callback_t)(uint32_t*, uint32_t*);

        frame_ready_callback_t frame_ready_cb = nullptr;

        typedef std::array <u8, 0x2000>     vram_bank_t;
        typedef std::array <u8, 0xa0>       oam_t;
        typedef std::array <sprite_t*, 40>  sprite_array_t;
        typedef std::array <u8, 0x40>       cgb_palette_ram_t;
        typedef std::array <u8, 0xc>        registers_t;
        typedef std::array <u32, 4>         dmg_palette_t;
        typedef std::array <vram_bank_t, 2> vram_t;
        typedef queue_t <fifo_pixel_t>      fifo_t;
        typedef std::vector <sprite_t>      queued_sprite_array_t;

        typedef lgw::framebuffer <PPU_WIDTH, PPU_HEIGHT> framebuffer_t;

        bool stat_blocking;
        queued_sprite_array_t queued_sprites;
        sprite_array_t sprites;
        vram_t vram;
        oam_t oam;

        u8 bcps, ocps,
           current_bank_idx = 0,
           dummy = 0;

        registers_t r = { 0 };
        cgb_palette_ram_t cgb_bg_palette = { 0 };
        cgb_palette_ram_t cgb_spr_palette = { 0 };

        fifo_t background_fifo,
               sprite_fifo;

        framebuffer_t frame[2];

        bool buffer_latch = false;

#ifdef GEEBLY_FRAMEBUFFER_FORMAT
#if GEEBLY_FRAMEBUFFER_FORMAT == 1
        dmg_palette_t dmg_palette = {
            0xfffffffful,
            0xffaaaaaaul,
            0xff555555ul,
            0xff000000ul
        };
#elif GEEBLY_FRAMEBUFFER_FORMAT == 2
        dmg_palette_t dmg_palette = {
            0xfffffffful,
            0xaaaaaafful,
            0x555555fful,
            0x000000fful
        };
#endif
#else
        dmg_palette_t dmg_palette = {
            0xfffffffful,
            0xffaaaaaaul,
            0xff555555ul,
            0xff000000ul
        };
#endif

        bool bg_auto_inc = false,
             spr_auto_inc = false;

        int clk = 0;
        size_t cx, rx, sx, sy, fx, clki = 4, wiy = 0;

        u16 tile_scy_off, tile_scx_off, coff;

        bool vram_disabled = false, oam_disabled = false;

        u8 tile, l, h;

        void set_frame_ready_callback(frame_ready_callback_t fr_cb) {
            frame_ready_cb = fr_cb;
        }

        uint32_t* get_buffer() {
            return frame[buffer_latch].get_buffer();
        }

        void reset() {
            clk = 0;
            cx = 0;
            rx = 0;
            sx = 0;
            sy = 0;
            fx = 0;
            clki = 0;
            wiy = 0;
            tile_scy_off = 0;
            tile_scx_off = 0;
            coff = 0;
            vram_disabled = false;
            oam_disabled = false;
            tile = false;
            l = 0;
            h = 0;

            queued_sprites.clear();
            queued_sprites.reserve(10);

            bcps = 0;
            ocps = 0;
            current_bank_idx = 0;

            // Flush FIFOs
            while (!background_fifo.empty()) background_fifo.pop();
            while (!sprite_fifo.empty()) sprite_fifo.pop();

            r[PPU_STAT] &= 0xf8;
        }

        void save_state(std::ofstream& o) {
            GEEBLY_WRITE_VARIABLE(clk);
            GEEBLY_WRITE_VARIABLE(cx);
            GEEBLY_WRITE_VARIABLE(rx);
            GEEBLY_WRITE_VARIABLE(sx);
            GEEBLY_WRITE_VARIABLE(sy);
            GEEBLY_WRITE_VARIABLE(fx);
            GEEBLY_WRITE_VARIABLE(clki);
            GEEBLY_WRITE_VARIABLE(wiy);
            GEEBLY_WRITE_VARIABLE(tile_scy_off);
            GEEBLY_WRITE_VARIABLE(tile_scx_off);
            GEEBLY_WRITE_VARIABLE(coff);
            GEEBLY_WRITE_VARIABLE(vram_disabled);
            GEEBLY_WRITE_VARIABLE(oam_disabled);
            GEEBLY_WRITE_VARIABLE(tile);
            GEEBLY_WRITE_VARIABLE(l);
            GEEBLY_WRITE_VARIABLE(h);
            GEEBLY_WRITE_VARIABLE(bg_auto_inc);
            GEEBLY_WRITE_VARIABLE(spr_auto_inc);
            GEEBLY_WRITE_VARIABLE(bcps);
            GEEBLY_WRITE_VARIABLE(ocps);
            GEEBLY_WRITE_VARIABLE(current_bank_idx);

            for (vram_bank_t& b : vram)
                o.write(reinterpret_cast<char*>(b.data()), b.size());
            
            o.write(reinterpret_cast<char*>(oam.data()), oam.size());
            o.write(reinterpret_cast<char*>(r.data()), r.size());
            o.write(reinterpret_cast<char*>(r.data()), r.size());
            o.write(reinterpret_cast<char*>(cgb_bg_palette.data()), cgb_bg_palette.size());
            o.write(reinterpret_cast<char*>(cgb_spr_palette.data()), cgb_spr_palette.size());
            //o.write(queued_sprites.data(), queued_sprites.size());

            //o.write(background_fifo.data(), queued_sprites.size());
            //o.write(queued_sprites.data(), queued_sprites.size());
        }
        
        void load_state(std::ifstream& i) {
            GEEBLY_LOAD_VARIABLE(clk);
            GEEBLY_LOAD_VARIABLE(cx);
            GEEBLY_LOAD_VARIABLE(rx);
            GEEBLY_LOAD_VARIABLE(sx);
            GEEBLY_LOAD_VARIABLE(sy);
            GEEBLY_LOAD_VARIABLE(fx);
            GEEBLY_LOAD_VARIABLE(clki);
            GEEBLY_LOAD_VARIABLE(wiy);
            GEEBLY_LOAD_VARIABLE(tile_scy_off);
            GEEBLY_LOAD_VARIABLE(tile_scx_off);
            GEEBLY_LOAD_VARIABLE(coff);
            GEEBLY_LOAD_VARIABLE(vram_disabled);
            GEEBLY_LOAD_VARIABLE(oam_disabled);
            GEEBLY_LOAD_VARIABLE(tile);
            GEEBLY_LOAD_VARIABLE(l);
            GEEBLY_LOAD_VARIABLE(h);
            GEEBLY_LOAD_VARIABLE(bg_auto_inc);
            GEEBLY_LOAD_VARIABLE(spr_auto_inc);
            GEEBLY_LOAD_VARIABLE(bcps);
            GEEBLY_LOAD_VARIABLE(ocps);
            GEEBLY_LOAD_VARIABLE(current_bank_idx);

            for (vram_bank_t& b : vram)
                i.read(reinterpret_cast<char*>(b.data()), b.size());
            
            i.read(reinterpret_cast<char*>(oam.data()), oam.size());
            i.read(reinterpret_cast<char*>(r.data()), r.size());
            i.read(reinterpret_cast<char*>(r.data()), r.size());
            i.read(reinterpret_cast<char*>(cgb_bg_palette.data()), cgb_bg_palette.size());
            i.read(reinterpret_cast<char*>(cgb_spr_palette.data()), cgb_spr_palette.size());
            //o.write(queued_sprites.data(), queued_sprites.size());

            //o.write(background_fifo.data(), queued_sprites.size());
            //o.write(queued_sprites.data(), queued_sprites.size());
        }

        void reset_full() {
            reset();

            for (vram_bank_t& b : vram)
               for (u8& v : b) v = 0;

            for (u8& v : oam) v = 0;
            for (u8& v : r) v = 0;
            for (u8& v : cgb_bg_palette) v = 0;
            for (u8& v : cgb_spr_palette) v = 0;
        }

        void reset_fetcher_state() {
            r[PPU_LY] = 0x0;
            r[PPU_STAT] &= ~STAT_CRMODE;
        }

        u32 read(u16 addr, size_t size) {
            switch (addr) {
                case MMIO_VBK: return settings::cgb_mode ? (0xfe | current_bank_idx) : 0xff;
                case MMIO_BCPD: return settings::cgb_mode ? utility::default_mb_read(cgb_bg_palette.data(), bcps & 0x3f, size) : 0xff;
            }
            
            if (addr >= PPU_R_BEGIN && addr <= PPU_R_END) {
                if (addr == 0xff41) return 0x80 | r[PPU_STAT];

                return utility::default_mb_read(r.data(), addr, size, PPU_R_BEGIN);
            }

            if (addr >= VRAM_BEGIN && addr <= VRAM_END) {
                if (oam_disabled && TEST_REG(PPU_LCDC, LCDC_SWITCH) && settings::vram_access_emulation_enabled) return 0xff;
                return utility::default_mb_read(vram[current_bank_idx].data(), addr, size, VRAM_BEGIN);
            }

            if (addr >= OAM_BEGIN && addr <= OAM_END) {
                if (oam_disabled && TEST_REG(PPU_LCDC, LCDC_SWITCH)) { for (auto& b : oam) b = rand() % 0xff; return 0xff; }
                return utility::default_mb_read(oam.data(), addr, size, OAM_BEGIN);
            }

            return 0xff;
        }

        void write(u16 addr, u16 value, size_t size) {
            if (addr == 0xff41) { if (pause) _log(debug, "value=%02x, stat=%02x", value, r[PPU_STAT]); value &= 0xf8; }

            switch (addr) {
                case 0xff40: { if (!(value & 0x80)) reset(); } break;
                case 0xff41: {} break;
                case MMIO_VBK: { current_bank_idx = value & 0x1; return; };
                case MMIO_BCPS: { bcps = value; return; }
                case MMIO_BCPD: {
                    utility::default_mb_write(cgb_bg_palette.data(), bcps & 0x3f, value, size, 0);

                    if (bcps & 0x80) bcps = ((bcps + 1) & 0xbf);

                    return;
                }
                case MMIO_OCPS: { ocps = value; return; }
                case MMIO_OCPD: {
                    utility::default_mb_write(cgb_spr_palette.data(), ocps & 0x3f, value, size, 0);

                    if (ocps & 0x80) ocps = ((ocps + 1) & 0xbf);

                    return;
                }
            }

            if (addr >= PPU_R_BEGIN && addr <= PPU_R_END) {
                // Ignore writes to LY
                if (addr == 0xff44) return;

                u8 old_stat = r[PPU_STAT];
                utility::default_mb_write(r.data(), addr, value, size, PPU_R_BEGIN);

                if (addr == 0xff41) r[PPU_STAT] |= old_stat & 0x7;

                return;
            }

            if (addr >= VRAM_BEGIN && addr <= VRAM_END) {
                if (vram_disabled && TEST_REG(PPU_LCDC, LCDC_SWITCH) && settings::vram_access_emulation_enabled) return;
                utility::default_mb_write(vram[current_bank_idx].data(), addr, value, size, VRAM_BEGIN);
                return;
            }

            if (addr >= OAM_BEGIN && addr <= OAM_END) {
                // if (oam_disabled && TEST_REG(PPU_LCDC, LCDC_SWITCH)) { _log(debug, "corrupting OAM..."); for (auto& b : oam) b = rand() % 0xff; return; }
                utility::default_mb_write(oam.data(), addr, value, size, OAM_BEGIN);
                return;
            }
        }

        u8& ref(u16 addr) {
            switch (addr) {
                case MMIO_VBK: return current_bank_idx;
            }

            if (addr >= PPU_R_BEGIN && addr <= PPU_R_END) return r[addr-PPU_R_BEGIN];
            
            if (addr >= VRAM_BEGIN && addr <= VRAM_END) return vram[current_bank_idx].at(addr-VRAM_BEGIN);

            if (addr >= OAM_BEGIN && addr <= OAM_END) return oam[addr-OAM_BEGIN];

            return dummy;
        }
    }
}