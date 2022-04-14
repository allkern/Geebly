#pragma once

#include "../mapper.hpp"

#include "../../../global.hpp"
#include "common.hpp"
#include "dsp.hpp"

#include <cmath>

namespace gameboy {
    namespace cart {
        class aec1 : public mapper {
            u8 dummy = 0;

            typedef std::array<u8, 0x7eb0> rom_t;

            rom_t rom = { 0 };

        public:
            inline bool vin_line_connected() override {
                return true;
            }

            double t = 0.0;

            struct channel_t {
                u8 control;
                u8 master_volume;

                struct operator_t {
                    struct latched_data_t {
                        u16 value;
                        bool latch;
                    };

                    latched_data_t frequency;
                    u8 amplitude;
                    latched_data_t adsr_attack;
                    latched_data_t adsr_decay;
                    latched_data_t adsr_sustain;
                    latched_data_t adsr_release;
                    latched_data_t adsr_base_level;
                    latched_data_t adsr_peak_level;
                    latched_data_t adsr_sustain_level;
                } operators_data[4];
            };

            u8 handle_latched_access(channel_t::operator_t::latched_data_t& data, bool write = false, u8 value = 0x0) {
                if (write) {
                    if (data.latch) {
                        data.value &= 0xff;
                        data.value |= value << 8;
                    } else {
                        data.value &= 0xff00;
                        data.value |= value;
                    }

                    data.latch = !data.latch;
                } else {
                    u8 r = 0x0;

                    if (data.latch) {
                        r = (data.value >> 8) & 0xff;
                    } else {
                        r = data.value & 0xff;
                    }

                    data.latch = !data.latch;

                    return r;
                }

                return 0x0;
            }

            channel_t channel_data[8];
            fm_channel_t channels[8];

            void update_channels() {
                for (int c = 0; c < 8; c++) {
                    if (channel_data[0].master_volume) {
                        if (channel_data[0].master_volume == 0xff) {
                            channels[c].a = 1.0;
                        } else {
                            channels[c].a = 1.0 / (255.0 - (double)channel_data[0].master_volume);
                        }
                    } else {
                        channels[c].a = 0.0;
                    }
                    
                    if (channel_data[c].control & 0x10) channels[c].a = 0.0;

                    for (int o = 0; o < 4; o++) {
                        channels[c].operators[o].enabled = channel_data[c].control & (1 << o);
                        channels[c].operators[o].f = 131072.0 / (2048.0 - (double)channel_data[c].operators_data[o].frequency.value);
                        channels[c].operators[o].adsr.a = (double)channel_data[c].operators_data[o].adsr_attack.value;
                        channels[c].operators[o].adsr.d = (double)channel_data[c].operators_data[o].adsr_decay.value;
                        channels[c].operators[o].adsr.s = (double)channel_data[c].operators_data[o].adsr_sustain.value;
                        channels[c].operators[o].adsr.r = (double)channel_data[c].operators_data[o].adsr_release.value;
                        channels[c].operators[o].adsr.base_level = (double)channel_data[c].operators_data[o].adsr_base_level.value;
                        channels[c].operators[o].adsr.peak_level = (double)channel_data[c].operators_data[o].adsr_peak_level.value;
                        channels[c].operators[o].adsr.sustain_level = (double)channel_data[c].operators_data[o].adsr_sustain_level.value;
                    }
                }
                // _log(debug,
                //     "channel[0].a=%f\n"
                //     "channel[0].operator[3].enabled=%u\n"
                //     "channel[0].operator[2].enabled=%u\n"
                //     "channel[0].operator[3].f=%f\n"
                //     "channel[0].operator[3].adsr.a=%f\n"
                //     "channel[0].operator[3].adsr.d=%f\n"
                //     "channel[0].operator[3].adsr.s=%f\n"
                //     "channel[0].operator[3].adsr.r=%f\n"
                //     "channel[0].operator[3].adsr.peak_level=%f\n"
                //     "channel[0].operator[3].adsr.sustain_level=%f\n"
                //     "channel[0].operator[3].adsr.base_level=%f\n",
                //     channels[0].a,
                //     channels[0].operators[3].enabled,
                //     channels[0].operators[2].enabled,
                //     channels[0].operators[3].f,
                //     channels[0].operators[3].adsr.a,
                //     channels[0].operators[3].adsr.d,
                //     channels[0].operators[3].adsr.s,
                //     channels[0].operators[3].adsr.r,
                //     channels[0].operators[3].adsr.peak_level,
                //     channels[0].operators[3].adsr.sustain_level,
                //     channels[0].operators[3].adsr.base_level
                // );
            }

            int16_t get_sample() override {
                double sample = 0.0;

                for (int i = 0; i < 8; i++) {
                    sample += channels[i].get_sample(t);
                }

                sample /= 8.0;

                t += 1.0;

                return sample * (2.0 * 0x7fff);
            }

            void save_state(std::ofstream& o) override {}
            void load_state(std::ifstream& i) override {}

            aec1(bool has_sram = false, std::ifstream* sav = nullptr) {}

            u8* get_bank0() { return rom.data(); }
            u8* get_bank1() { return &rom[0x3eaf]; }
            u8* get_sram() { return nullptr; }

            bool save_sram(std::ofstream& sav) override {
                return false;
            }

            void init(std::ifstream* f) override {
                tag = mapper_tag::aec1;
                
                if (f->is_open() && f->good()) {
                    f->read((char*)rom.data(), rom.size());
                }
                
                f->close();
            }

            void restart_channel_adsr(int channel) {
                for (int i = 0; i < 4; i++) {
                    channels[channel].operators[i].adsr.m_samples = 0;
                    channels[channel].operators[i].adsr.state = fm_channel_t::fm_operator_t::AS_NONE;
                }
            }

            void write(u16 addr, u16 value, size_t size) override {
                if (addr >= 0xb000) {
                    switch (addr) {
                        case 0xb000: {
                            channel_data[(value >> 4) & 0x7] = channel_data[value & 0x7];
                        } break;
                        
                        case 0xb001: {
                            channel_data[value & 0x7].operators_data[(value >> 5) & 0x3] = channel_data[value & 0x7].operators_data[(value >> 3) & 0x3];
                        } break;
                    }
                }
                int channel = (addr >> 8) & 0x7,
                    operator_id = (addr >> 4) & 0x3;
                
                switch (addr & 0xf) {
                    case 0x00: { // Channel control
                        channel_data[channel].control = value;

                        if (value & 0x80) restart_channel_adsr(channel);
                    } break;
                    case 0x01: { // Channel master volume
                        channel_data[channel].master_volume = value;
                    } break;
                    case 0x02: {
                        handle_latched_access(
                            channel_data[channel].operators_data[operator_id].frequency,
                            true,
                            value
                        );
                    } break;
                    case 0x03: {
                        channel_data[channel].operators_data[operator_id].amplitude = value;
                    } break;
                    case 0x04: {
                        handle_latched_access(
                            channel_data[channel].operators_data[operator_id].adsr_attack,
                            true,
                            value
                        );
                    } break;
                    case 0x05: {
                        handle_latched_access(
                            channel_data[channel].operators_data[operator_id].adsr_decay,
                            true,
                            value
                        );
                    } break;
                    case 0x06: {
                        handle_latched_access(
                            channel_data[channel].operators_data[operator_id].adsr_sustain,
                            true,
                            value
                        );
                    } break;
                    case 0x07: {
                        handle_latched_access(
                            channel_data[channel].operators_data[operator_id].adsr_release,
                            true,
                            value
                        );
                    } break;
                    case 0x08: {
                        handle_latched_access(
                            channel_data[channel].operators_data[operator_id].adsr_base_level,
                            true,
                            value
                        );
                    } break;
                    case 0x09: {
                        handle_latched_access(
                            channel_data[channel].operators_data[operator_id].adsr_peak_level,
                            true,
                            value
                        );
                    } break;
                    case 0x0a: {
                        handle_latched_access(
                            channel_data[channel].operators_data[operator_id].adsr_sustain_level,
                            true,
                            value
                        );
                    } break;
                }

                update_channels();
            };

            u32 read(u16 addr, size_t size) override {
                if (addr >= 0x150 && addr <= 0x7fff) { return utility::default_mb_read(rom.data(), addr, size, ROM_BEGIN); }
                if (addr >= 0xa000 && addr <= 0xafff) {
                    int channel = (addr >> 8) & 0x7,
                        operator_id = (addr >> 4) & 0x3;
                    
                    switch (addr & 0xf) {
                        case 0x00: { return channel_data[channel].control; }
                        case 0x01: { return channel_data[channel].master_volume; }
                    }
                }
                return 0xff;
            }

            u8& ref(u16 addr) override {
                if (addr >= 0x150 && addr <= 0x7fff) { return rom.at(addr-0x150); }

                return dummy;
            }
        };
    }
}

#undef SPU_NATIVE_SAMPLERATE
#undef M_PI