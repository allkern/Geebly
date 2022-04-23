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
                u8 algorithm_lfo_enable;
                u8 lfo_freq, lfo_amp;

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
                    u8 multiplier, detune;
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
                            channels[c].main_amp = 1.0;
                        } else {
                            channels[c].main_amp = 1.0 / (255.0 - (double)channel_data[0].master_volume);
                        }
                    } else {
                        channels[c].main_amp = 0.0;
                    }
                    
                    if (channel_data[c].control & 0x10) channels[c].main_amp = 0.0;

                    channels[c].algorithm = (fm_channel_t::algorithm_t)(channel_data[c].algorithm_lfo_enable & 0x7f);
                    channels[c].lfo_enable = channel_data[c].algorithm_lfo_enable & 0x80;
                    channels[c].lfo.f = ((double)channel_data[c].lfo_freq) / 10.0; // 0.0 -> 25.5 Hz LFO freqs
                    channels[c].lfo.a = ((double)channel_data[c].lfo_amp) / 10.0; // 0.0 -> 25.5 LFO amps

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
                        
                        if (!channel_data[c].operators_data[o].multiplier) channel_data[c].operators_data[o].multiplier = 1;

                        double multiplier = (double)(channel_data[c].operators_data[o].multiplier & 0x7f);
                        bool reciprocal = channel_data[c].operators_data[o].multiplier & 0x80;

                        channels[c].operators[o].multiplier = reciprocal ? (1.0 / multiplier) : multiplier;
                        channels[c].operators[o].detune = ((double)channel_data[c].operators_data[o].detune) / 10.0; // 0.0 -> 25.5 Hz detune
                    }
                }
            }

            int16_t get_sample() override {
                double sample = 0.0;

                for (int i = 0; i < 8; i++) {
                    if ((int)channels[i].main_amp) {
                        sample += channels[i].get_sample();
                    }
                }

                sample /= 8.0;

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
                    channels[channel].t = 0.0;
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
                            // _log(debug, "Copying channel %u operator %u to operator %u", value & 0x7, (value >> 3) & 0x3, (value >> 5) & 0x3);
                            channel_data[value & 0x7].operators_data[(value >> 5) & 0x3] = channel_data[value & 0x7].operators_data[(value >> 3) & 0x3];
                        } break;
                    }
                    return;
                }

                int channel = (addr >> 8) & 0x7,
                    operator_id = (addr >> 4) & 0x3;
                
                switch (addr & 0xf) {
                    case 0x00: { // Channel control
                        channel_data[channel].control = value;
                        
                        // _log(debug, "Channel %u Control write %02x", channel, value);

                        if (value & 0x80) {
                            update_channels();

                            // _log(debug, "Playing channel %u with settings:\n\t"
                            //     "channel.a=%f\n\t",
                            //     channel,
                            //     channels[channel].main_amp
                            // );

                            // for (int i = 0; i < 4; i++) {
                            //     _log(debug,
                            //         "Operator %u:\n\t"
                            //         "enabled=%u\n\t"
                            //         "f=%f\n\t"
                            //         "a=%f\n\t"
                            //         "adsr.enabled=%u\n\t"
                            //         "adsr.a=%f\n\t"
                            //         "adsr.d=%f\n\t"
                            //         "adsr.s=%f\n\t"
                            //         "adsr.r=%f\n\t"
                            //         "adsr.peak_level=%f\n\t"
                            //         "adsr.sustain_level=%f\n\t"
                            //         "adsr.base_level=%f\n\t",
                            //         i + 1,
                            //         channels[channel].operators[i].enabled,
                            //         channels[channel].operators[i].f,
                            //         channels[channel].operators[i].a,
                            //         channels[channel].operators[i].adsr.enabled,
                            //         channels[channel].operators[i].adsr.a,
                            //         channels[channel].operators[i].adsr.d,
                            //         channels[channel].operators[i].adsr.s,
                            //         channels[channel].operators[i].adsr.r,
                            //         channels[channel].operators[i].adsr.peak_level,
                            //         channels[channel].operators[i].adsr.sustain_level,
                            //         channels[channel].operators[i].adsr.base_level
                            //     );
                            // }

                            restart_channel_adsr(channel);
                        }
                    } break;
                    case 0x01: { // Channel master volume
                        channel_data[channel].master_volume = value;
                        //_log(debug, "Channel %u Master Volume write %02x", channel, value);
                    } break;
                    case 0x02: {
                        handle_latched_access(
                            channel_data[channel].operators_data[operator_id].frequency,
                            true,
                            value
                        );
                        // _log(debug, "Channel %u operator %u frequency write %02x: value=%04x",
                        //     channel,
                        //     operator_id,
                        //     value,
                        //     channel_data[channel].operators_data[operator_id].frequency.value
                        // );
                    } break;
                    case 0x03: {
                        channel_data[channel].operators_data[operator_id].amplitude = value;
                        // _log(debug, "Channel %u operator %u amplitude write %02x: value=%02x",
                        //     channel,
                        //     operator_id,
                        //     value,
                        //     channel_data[channel].operators_data[operator_id].amplitude
                        // );
                    } break;
                    case 0x04: {
                        handle_latched_access(
                            channel_data[channel].operators_data[operator_id].adsr_attack,
                            true,
                            value
                        );
                        // _log(debug, "Channel %u operator %u ADSR attack write %02x: value=%04x",
                        //     channel,
                        //     operator_id,
                        //     value,
                        //     channel_data[channel].operators_data[operator_id].adsr_attack.value
                        // );
                    } break;
                    case 0x05: {
                        handle_latched_access(
                            channel_data[channel].operators_data[operator_id].adsr_decay,
                            true,
                            value
                        );
                        // _log(debug, "Channel %u operator %u ADSR decay write %02x: value=%04x",
                        //     channel,
                        //     operator_id,
                        //     value,
                        //     channel_data[channel].operators_data[operator_id].adsr_decay.value
                        // );
                    } break;
                    case 0x06: {
                        handle_latched_access(
                            channel_data[channel].operators_data[operator_id].adsr_sustain,
                            true,
                            value
                        );
                        // _log(debug, "Channel %u operator %u ADSR sustain write %02x: value=%04x",
                        //     channel,
                        //     operator_id,
                        //     value,
                        //     channel_data[channel].operators_data[operator_id].adsr_sustain.value
                        // );
                    } break;
                    case 0x07: {
                        handle_latched_access(
                            channel_data[channel].operators_data[operator_id].adsr_release,
                            true,
                            value
                        );
                        // _log(debug, "Channel %u operator %u ADSR release write %02x: value=%04x",
                        //     channel,
                        //     operator_id,
                        //     value,
                        //     channel_data[channel].operators_data[operator_id].adsr_release.value
                        // );
                    } break;
                    case 0x08: {
                        handle_latched_access(
                            channel_data[channel].operators_data[operator_id].adsr_base_level,
                            true,
                            value
                        );
                        // _log(debug, "Channel %u operator %u ADSR base level write %02x: value=%04x",
                        //     channel,
                        //     operator_id,
                        //     value,
                        //     channel_data[channel].operators_data[operator_id].adsr_base_level.value
                        // );
                    } break;
                    case 0x09: {
                        handle_latched_access(
                            channel_data[channel].operators_data[operator_id].adsr_peak_level,
                            true,
                            value
                        );
                        // _log(debug, "Channel %u operator %u ADSR peak level write %02x: value=%04x",
                        //     channel,
                        //     operator_id,
                        //     value,
                        //     channel_data[channel].operators_data[operator_id].adsr_peak_level.value
                        // );
                    } break;
                    case 0x0a: {
                        handle_latched_access(
                            channel_data[channel].operators_data[operator_id].adsr_sustain_level,
                            true,
                            value
                        );
                        // _log(debug, "Channel %u operator %u ADSR sustain level write %02x: value=%04x",
                        //     channel,
                        //     operator_id,
                        //     value,
                        //     channel_data[channel].operators_data[operator_id].adsr_sustain_level.value
                        // );
                    } break;
                    case 0x0b: {
                        channel_data[channel].operators_data[operator_id].multiplier = value;
                    } break;
                    case 0x0c: {
                        channel_data[channel].operators_data[operator_id].detune = value;
                    } break;
                    case 0x0d: {
                        channel_data[channel].algorithm_lfo_enable = value;
                    } break;
                    case 0x0e: {
                        channel_data[channel].lfo_freq = value;
                    } break;
                    case 0x0f: {
                        channel_data[channel].lfo_amp = value;
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