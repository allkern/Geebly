#pragma once

#include "../../aliases.hpp"
#include "../../global.hpp"
#include "../../log.hpp"

#include "memory.hpp"
#include "square.hpp"
#include "noise.hpp"
#include "wave.hpp"

namespace gameboy {
    namespace spu {
        int32_t get_sample_naive() {
            int16_t s1 = ch1.get_sample(),
                    s2 = ch2.get_sample(),
                    s3 = ch3.get_sample(),
                    s4 = ch4.get_sample(),
                    ls1 = s1,
                    ls2 = s2,
                    ls3 = s3,
                    ls4 = s4,
                    left, right;

            if (false) {
                if (!(nr[0x15] & 0x10)) ls1 = 0;
                if (!(nr[0x15] & 0x20)) ls2 = 0;
                if (!(nr[0x15] & 0x40)) ls3 = 0;
                if (!(nr[0x15] & 0x80)) ls4 = 0;

                left = (((ls1 + ls2 + ls3 + ls4) / 4) * so1_output_level) * master_volume;

                ls1 = s1;
                ls2 = s2;
                ls3 = s3;
                ls4 = s4;

                if (!(nr[0x15] & 0x1)) ls1 = 0;
                if (!(nr[0x15] & 0x2)) ls2 = 0;
                if (!(nr[0x15] & 0x4)) ls3 = 0;
                if (!(nr[0x15] & 0x8)) ls4 = 0;

                right = (((ls1 + ls2 + ls3 + ls4) / 4) * so2_output_level) * master_volume;
            } else {
                left = ((ls1 + ls2 + ls3 + ls4) / 4);
                right = left;
            }

            return ((right & 0xffff) << 0x10) | (left & 0xffff);
        }

        inline int16_t clamp(int32_t v) { return (v > 0xffff) ? 0xffff : v; }

        int32_t get_sample() {
            int16_t samples[] = {
                ch1.get_sample(),
                ch2.get_sample(),
                ch3.get_sample(),
                ch4.get_sample()
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

            return (((int16_t)right & 0xffff) << 0x10) | ((int16_t)left & 0xffff);
        }

        void init() {
            or_mask[0x00] = 0x80;
            or_mask[0x01] = 0x3f;
            or_mask[0x04] = 0xb8;
            or_mask[0x05] = 0xff;
            or_mask[0x06] = 0x3f;
            or_mask[0x09] = 0xb8;
            or_mask[0x0a] = 0x7f;
            or_mask[0x0b] = 0xff;
            or_mask[0x0c] = 0x9f;
            or_mask[0x0e] = 0xb8;
            or_mask[0x0f] = 0xff;
            or_mask[0x10] = 0xff;
            or_mask[0x13] = 0xbf;
            or_mask[0x16] = 0x70;

            ch1.init(nr[0x0]);
            ch2.init(nr[0x5]);
            ch3.init(nr[0xa]);
            ch4.init(nr[0xf]);
        }

        void reset(bool reset_on = true) {
            if (reset_on) spu_on = ~spu_on;

            ch1.reset();
            ch2.reset();
            ch3.reset();
            ch4.reset();
        }

        void write(u16 addr, u16 value, size_t size) {
            // Handle a write to Wave RAM
            if (addr >= 0xff30 && addr <= 0xff3f) {
                utility::default_mb_write(wave_ram.data(), addr, value, size, SPU_BEGIN+0x20);
                return;
            }

            if ((addr >= 0xff10) && (addr <= 0xff25) && (!(nr[0x16] & 0x80))) return;

            utility::default_mb_write(nr.data(), addr, value, size, SPU_BEGIN);

            if (sound_disabled) return;
    
            if (addr == 0xff14) ch1.update_state();
            if (addr == 0xff19) ch2.update_state();
            if (addr == 0xff1e) ch3.update_state();
            if (addr == 0xff23) ch4.update_state();

            // To-do: Improve ch3 output level emulation
            //        Overall better SPU accuracy needed (using DIV as timing source?)
            if (addr == 0xff1c) ch3.update_output_level();

            if (addr == 0xff24) {
                so1_output_level = (double)(nr[0x14] & 0x7) / 7;
                so2_output_level = (double)((nr[0x14] >> 4) & 0x7) / 7;
            }

            if (addr == 0xff26) {
                if ((!(value & 0x80) && (nr[0x16] & 0x80)) || ((value & 0x80) && (!(nr[0x16] & 0x80)))) reset();

                nr[0x16] = value & 0x80;
            }
        }

        u32 read(u16 addr, size_t size) {
            if (addr >= 0xff10 && addr <= 0xff26) {
                return utility::default_mb_read(nr.data(), addr, size, SPU_BEGIN) | or_mask.at(addr - SPU_BEGIN);
            }

            if (addr >= 0xff30 && addr <= 0xff3f) {
                return utility::default_mb_read(wave_ram.data(), addr, size, SPU_BEGIN+0x20);
            }

            return 0xff;
        }
    }
}