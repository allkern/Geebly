#pragma once

#define AEC1A_SAMPLERATE 2097152
#define AEC1A_PI         3.14159265358979323846264338327950288

#include <cmath>

#ifdef AEC1A_FAST_DSP
#ifdef AEC1A_ENABLE_ALL_OPTS
#ifndef AEC1A_USE_FAST_FLOOR
#define AEC1A_USE_FAST_FLOOR
#endif
#ifndef AEC1A_USE_FAST_ABS
#define AEC1A_USE_FAST_ABS
#endif
#endif
#define AEC1A_DSP_TYPE float
#ifdef AEC1A_USE_FAST_FLOOR
inline AEC1A_DSP_TYPE __fast_floor(AEC1A_DSP_TYPE x) {
    AEC1A_DSP_TYPE f = (AEC1A_DSP_TYPE)(int)x;

    return f - (f > x);
}
#define AEC1A_FLOOR __fast_floor
#else
#define AEC1A_FLOOR std::floor
#endif
// #ifdef AEC1A_USE_FAST_ABS
// inline float __fast_abs(register float n) {
//     // asm(R"(
//     //     movd %xmm0, %eax
//     //     and $0x7fffffff, %eax
//     // )");
    
//     uint32_t p = (*reinterpret_cast<uint32_t*>(&n)) & 0x7fffffff;

//     return *reinterpret_cast<float*>(&p);
// }
// #define AEC1A_ABS __fast_abs
// #else
#define AEC1A_ABS std::abs
//#endif
inline AEC1A_DSP_TYPE __fast_sin(AEC1A_DSP_TYPE x) noexcept {
    constexpr AEC1A_DSP_TYPE tp = 1.0f/(2.0f*AEC1A_PI);
    x *= tp;
    x -= 0.25f + AEC1A_FLOOR(x + 0.25f);
    x *= 16.0f * (AEC1A_ABS(x) - 0.5f);
    #ifdef AEC1A_EXTRA_PRECISION
    x += 0.225f * x * (AEC1A_ABS(x) - 1.0f);
    #endif
    return x;
}
#define AEC1A_SIN __fast_sin
#else
#define AEC1A_DSP_TYPE double
#define AEC1A_SIN std::sin
#define AEC1A_FLOOR std::floor
#define AEC1A_ABS std::abs
#endif