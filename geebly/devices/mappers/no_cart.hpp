#pragma once

#include "mapper.hpp"

namespace gameboy {
    namespace cart {
        class no_cart : public mapper {
        public:
            void init(std::ifstream* f) override {
                tag = mapper_tag::no_cart;
            }

            void save_state(std::ofstream& o) override {}
            void load_state(std::ifstream& i) override {}

            u32 read(u16 addr, size_t size) override {
                return 0xff;
            }
        };
    }
}
