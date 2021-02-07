#pragma once

#include <atomic>

#define _TTAS_LOCK_OPTIMIZATION

namespace gameboy {
    namespace utility {
        struct ttas_lock {
        private:
            std::atomic<bool> lock_ = { false };

        public:
            void unlock() { lock_.store(false, std::memory_order_release); }

            void lock() {
                while (true) {
                    if (!lock_.exchange(true, std::memory_order_acquire)) {
                        break;
                    }

                    #ifdef _TTAS_LOCK_OPTIMIZATION
                        while (lock_.load(std::memory_order_relaxed)) {
                            __builtin_ia32_pause();
                        }
                    #else
                        while (lock_.load(std::memory_order_relaxed));
                    #endif
                }
            }
        };
    }
}