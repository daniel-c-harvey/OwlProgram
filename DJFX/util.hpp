
#pragma once
#include "basicmaths.h"

// TB-303 style tanh approximation for feedback saturation
static inline float tb303_tanh(float x)
{
    if (x > 3.f) return 1.f;
    if (x < -3.f) return -1.f;
    const float x2 = x * x;
    return x * (27.f + x2) / (27.f + 9.f * x2);
}

#ifndef param_val_to_f32 
#define param_val_to_f32(val) ((uint16_t)val * 9.77517106549365e-004f)
#endif

/// @brief Tunable logistic function (sigmoid)
/// @param a slope
/// @param b slope 2
/// @param c offset
/// @param z portion scalar
/// @return H(x)
static inline float H(float x, float a, float b, float c, float z)
{
    return z * a / (a + fast_expf(b * (c - x))) - 0.02f;
}