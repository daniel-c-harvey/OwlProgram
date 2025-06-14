
#pragma once
#include "basicmaths.h"

static inline float tanh_saturate(float x, float min_val, float max_val, float a,float b)
{
    if (x > max_val) return 1.f;
    if (x < min_val) return -1.f;
    const float x2 = x * x;
    return x * (a + x2) / (a + b + x2);
}

// TB-303 style feedback saturation
// Hard saturation for filter feedback (handles large values)
static inline float feedback_saturate(float x)
{
    return tanh_saturate(x, -3.f, 3.f, 27.f, 9.f);
}

// Gentle saturation for audio signals (subtle, musical)
static inline float audio_saturate(float x)
{
    return tanh_saturate(x, -1.5f, 1.5f, 12.f, 3.f);
}

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