#pragma once

#include "../../aliases.hpp"
#include "../../global.hpp"
#include "../../log.hpp"

#include "memory.hpp"
#include "square.hpp"
#include "noise.hpp"
#include "wave.hpp"

#include <mutex>

namespace gameboy {
    namespace spu {
        bool mute_ch1 = false,
             mute_ch2 = false,
             mute_ch3 = false,
             mute_ch4 = false;

        inline int16_t clamp(int32_t v) { return (v > 0xffff) ? 0xffff : v; }

        int32_t get_sample() {
            int16_t samples[] = {
                mute_ch1 ? (int16_t)0 : ch1.get_sample(),
                mute_ch2 ? (int16_t)0 : ch2.get_sample(),
                mute_ch3 ? (int16_t)0 : ch3.get_sample(),
                mute_ch4 ? (int16_t)0 : ch4.get_sample()
            };

            int left, right;

            if (stereo) {
                int sl = 0, sr = 0;

                for (int i = 0; i < 4; i++) {
                    if (nr[0x15] & (0x10 << i)) sl += samples[i];
                    if (nr[0x15] & (0x1 << i)) sr += samples[i];
                }

                left = ((sl / 4) * so1_output_level) * master_volume;
                right = ((sr / 4) * so2_output_level) * master_volume;
            } else {
                int sum = 0;
        
                for (int i = 0; i < 4; i++) sum += samples[i];

                left = sum / 4;

                left *= ((so1_output_level + so2_output_level) / 2);
                left *= master_volume;

                right = left;
            }

            right -= 0x3fff;
            left -= 0x3fff;

            return (((int16_t)right & 0xffff) << 0x10) | ((int16_t)left & 0xffff);
        }

        void init() {
            or_mask[0x00] = 0x80;
            or_mask[0x01] = 0x3f;
            or_mask[0x03] = 0xff;
            or_mask[0x04] = 0xbf;
            or_mask[0x05] = 0xff;
            or_mask[0x06] = 0x3f;
            or_mask[0x08] = 0xff;
            or_mask[0x09] = 0xbf;
            or_mask[0x0a] = 0x7f;
            or_mask[0x0b] = 0xff;
            or_mask[0x0c] = 0x9f;
            or_mask[0x0d] = 0xff;
            or_mask[0x0e] = 0xbf;
            or_mask[0x0f] = 0xff;
            or_mask[0x10] = 0xff;
            or_mask[0x13] = 0xbf;
            or_mask[0x16] = 0x70;

            ch1.init(nr[0x0], 0);
            ch2.init(nr[0x5], 1);
            ch3.init(nr[0xa]);
            ch4.init(nr[0xf]);
        }

        void save_state(std::ofstream& o) {
            o.write(reinterpret_cast<char*>(nr.data()), nr.size());
            o.write(reinterpret_cast<char*>(wave_ram.data()), wave_ram.size());
            
            GEEBLY_WRITE_VARIABLE(so1_output_level);
            GEEBLY_WRITE_VARIABLE(so2_output_level);
            GEEBLY_WRITE_VARIABLE(mixed);

            ch1.save_state(o);
            ch2.save_state(o);
            ch3.save_state(o);
            ch4.save_state(o);
        }

        void load_state(std::ifstream& i) {
            i.read(reinterpret_cast<char*>(nr.data()), nr.size());
            i.read(reinterpret_cast<char*>(wave_ram.data()), wave_ram.size());
            
            GEEBLY_LOAD_VARIABLE(so1_output_level);
            GEEBLY_LOAD_VARIABLE(so2_output_level);
            GEEBLY_LOAD_VARIABLE(mixed);

            ch1.load_state(i);
            ch2.load_state(i);
            ch3.load_state(i);
            ch4.load_state(i);
        }

        void reset(bool reset_on = true) {
            if (reset_on) spu_on = !spu_on;

            //_log(debug, "before spu reset ff14=%02x", nr[0x4]);
            ch1.reset();
            ch2.reset();
            ch3.reset();
            ch4.reset();

            nr[0x14] = 0x0;
            nr[0x15] = 0x0;
            //_log(debug, "after spu reset ff14=%02x", nr[0x4]);
        }

        void write(u16 addr, u16 value, size_t size) {
            // Ignore writes to unmapped registers
            if (addr >= 0xff27 && addr <= 0xff2f) return;

            // Handle a write to Wave RAM
            if (addr >= 0xff30 && addr <= 0xff3f) {
                utility::default_mb_write(wave_ram.data(), addr, value, size, SPU_BEGIN+0x20);
                return;
            }

            if ((addr >= 0xff10) && (addr <= 0xff25) && (!(nr[0x16] & 0x80))) return;

            utility::default_mb_write(&nr[0], addr, value, size, SPU_BEGIN);

            if (sound_disabled) return;

            if ((addr >= 0xff10) && (addr <= 0xff15)) ch1.update();
            if ((addr >= 0xff16) && (addr <= 0xff19)) ch2.update();
            if ((addr >= 0xff1a) && (addr <= 0xff1e)) ch3.update();
            if ((addr >= 0xff20) && (addr <= 0xff23)) ch4.update();

            if (addr == 0xff14) ch1.update_state();
            if (addr == 0xff19) ch2.update_state();
            if (addr == 0xff1e) ch3.update_state();
            if (addr == 0xff23) ch4.update_state();

            // To-do: Improve ch3 output level emulation
            //        Overall better SPU accuracy needed (using DIV as timing source?)
            // if (addr == 0xff1c) ch3.update_output_level();

            if (addr == 0xff24) {
                so1_output_level = (double)(nr[0x14] & 0x7) / 7;
                so2_output_level = (double)((nr[0x14] >> 4) & 0x7) / 7;
            }

            if (addr == 0xff26) {
                if (value == 0x0) reset();

                nr[0x16] = value & 0x80;

                //if (value & 0x7f) _log(debug, "ff26 write=%02x, value=%02x", nr[0x16], value);
            }
        }

        void reset_master_clock() {
            master_clk = 0;
        }

        void update() {
            master_clk++;
        }

        u32 read(u16 addr, size_t size) {
            if (addr >= 0xff27 && addr <= 0xff2f) return 0xff;
            if (addr == 0xff26) {
                // _log(debug, "ff26 read=%02x",
                //        (((u8)ch2.cs.playing) << 1) |
                //        (((u8)ch3.cs.playing) << 2) |
                //        (((u8)ch4.cs.playing) << 3) | or_mask.at(addr - SPU_BEGIN));
                return nr[0x16] |
                       (((u8)ch1.cs.playing) << 0) |
                       (((u8)ch2.cs.playing) << 1) |
                       (((u8)ch3.cs.playing) << 2) |
                       (((u8)ch4.cs.playing) << 3) | or_mask.at(addr - SPU_BEGIN);
            }
            if (addr >= 0xff10 && addr <= 0xff26) {
                return utility::default_mb_read(nr.data(), addr, size, SPU_BEGIN) | or_mask.at(addr - SPU_BEGIN);
            }

            //_log(debug, "waveram read");
            if (addr >= 0xff30 && addr <= 0xff3f) {
                if (ch3.cs.playing) {
                    return 0xff;
                } else {
                    return utility::default_mb_read(wave_ram.data(), addr, size, SPU_BEGIN+0x20);
                }
            }

            return 0xff;
        }
    }
}