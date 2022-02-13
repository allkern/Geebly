#pragma once

#include "../aliases.hpp"
#include "../global.hpp"

#include <cstring>

namespace gameboy {
    namespace sgb {
        static const char* command_table[] = {
            "PAL01",
            "PAL23",
            "PAL03",
            "PAL12",
            "ATTR_BLK",
            "ATTR_LIN",
            "ATTR_DIV",
            "ATTR_CHR",
            "SOUND",
            "SOU_TRN",
            "PAL_SET",
            "PAL_TRN",
            "ATRC_EN",
            "TEST_EN",
            "ICON_EN",
            "DATA_SND",
            "DATA_TRN",
            "MLT_REQ",
            "JUMP",
            "CHR_TRN",
            "PCT_TRN",
            "ATTR_TRN",
            "ATTR_SET",
            "MASK_EN",
            "OBJ_TRN",
            "Invalid",
            "Invalid",
            "Invalid",
            "Invalid",
            "Invalid",
            "Invalid",
            "Invalid",
        };

        int counter = 0;
        u8 data[0x10];
        int index = 0;

        void save_state(std::ofstream& o) {
        }

        void load_state(std::ifstream& i) {
        }

        enum state_t {
            ST_NONE,
            ST_READING_PACKET,
            ST_READING_STOP
        } state = ST_NONE;

        bool ongoing_transfer = false;

        void handle_transfer_stage_end() {
            switch (state) {
                case ST_READING_PACKET: {
                    _log(debug, "command=%02x, instruction=%s (%02x), length=%u",
                        data[0],
                        command_table[(data[0] >> 3) & 0x1f],
                        (data[0] >> 3) & 0x1f,
                        data[0] & 0x7
                    );

                    if (((data[0] >> 3) & 0x1f) == 0x19) {
                        _log(warning, "SGB 19 COMMAND ENCOUNTERED! DOING SOME EXTREMELY IMPORTANT EMULATOR STUFF REALLY INTERNAL THINGS NO ONE WOULD EVER UNDERSTAND 0x58bc43a964c7938ac49b6a374b98a");
                    }

                    _log(debug, "data=%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x",
                        data[0],
                        data[1],
                        data[2],
                        data[3],
                        data[4],
                        data[5],
                        data[6],
                        data[7],
                        data[8],
                        data[9],
                        data[10],
                        data[11],
                        data[12],
                        data[13],
                        data[14],
                        data[15]
                    );

                    counter = 2;

                    state = ST_READING_STOP;
                } break;

                case ST_READING_STOP: {
                    ongoing_transfer = false;
                    state = ST_NONE;
                    counter = 0;
                } break;
            }
        }

        void update(bool p14, bool p15) {
            if (!counter) return;

            if (counter) {
                int bit = p15 ? 1 : (p14 ? 0 : 2);

                switch (state) {
                    case ST_READING_PACKET: {
                        u8& data_byte = data[(index >> 4) & 0xf];

                        if (p15) {
                            data_byte >>= 1;
                            data_byte |= 1 << 7;
                        } else if (p14) {
                            data_byte >>= 1;
                        }

                        index++;
                    } break;
                }
                
                counter--;

                // Transfer stage finished
                if (!counter)
                    handle_transfer_stage_end();
            }
        }

        bool transfer_ongoing() {
            return ongoing_transfer;
        }

        void start_transfer() {
            ongoing_transfer = true;

            counter = 0x100;
            index = 0;

            std::memset(&data[0], 0, sizeof(data));

            state = ST_READING_PACKET;
        }
    }
}