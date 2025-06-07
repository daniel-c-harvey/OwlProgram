#include "ui.hpp"
#include "basicmaths.h"
#include <cmath>

/** CONSTANTS */

static const float a1 = 2.f;
static const float a2 = 0.8f;
static const float b1 = 58.f;
static const float b2 = 8.f;
static const float c1 = 0.08125f;
static const float c2 = 0.425f;
static const float z1 = 0.36f;
static const float z2 = 0.3f;

static const uint8_t k_crossbands = 2;
static const float bands[k_crossbands][4] = {
    {a1, b1, c1, z1},
    {a2, b2, c2, z2}
};

/** /CONSTANTS */

static inline float H_HP_cutoff(float p_value)
{
    return powf(p_value, 0.62f) * 0.66f;
}

/// @brief Tunable logistic function (sigmoid)
/// @param a slope
/// @param b slope 2
/// @param c offset
/// @param z portion scalar
/// @return H(x)
static inline float HP_H(float x, float a, float b, float c, float z)
{
    return z * a / (a + expf(b * (c - x))) - 0.02f;
}

static inline float H_HP_reso(float x)
{
    float acc = 0;

    for (uint8_t band = 0; band < k_crossbands; band++)
    {
        acc += HP_H(x, bands[band][0], bands[band][1], bands[band][2], bands[band][3]);
    }

    return acc * 0.27f;
}

static inline float H_LP_cutoff(float p_value)
{
    return (1.f - (powf(p_value, 1.35f))) * 0.35f + 0.65f;
}

static inline float H_LP_reso(float x)
{
    return powf((1.f - cosf(M_PI * 2.f * (1.f - x))) / 2.f, 2.f) * 0.22f;
}

void UserParameters::setHP(float p_value)
{
    hp_params.p_cutoff = H_HP_cutoff(p_value);
    hp_params.p_resonance = H_HP_reso(p_value);
}

void UserParameters::setLP(float p_value)
{
    lp_params.p_cutoff = H_LP_cutoff(p_value);
    lp_params.p_resonance = H_LP_reso(p_value);
}

FilterParameters UserParameters::getHPParams()
{
    return hp_params;
}

FilterParameters UserParameters::getLPParams()
{
    return lp_params;
}
