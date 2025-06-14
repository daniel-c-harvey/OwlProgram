#pragma once

#include "basicmaths.h"
#include "butterworth.hpp"

template <int k_channels, typename TFeedbackLine, typename TCoefficients, typename TUIParams, typename TFilterParams>
inline FilterBase<k_channels, TFeedbackLine, TCoefficients, TUIParams, TFilterParams>::FilterBase(TFilterParams *p)
: params(p) {}

template <int k_channels, typename TFeedbackLine, typename TCoefficients, typename TUIParams, typename TFilterParams, typename TDecoratorParams>
FilterDecorator<k_channels, TFeedbackLine, TCoefficients, TUIParams, TFilterParams, TDecoratorParams>
::FilterDecorator(Filter<k_channels, TFeedbackLine, TCoefficients, TUIParams, TFilterParams> *f, TDecoratorParams *p)
: FilterBase<k_channels, TFeedbackLine, TCoefficients, TUIParams, TDecoratorParams>(p), filter_ptr(f) { }

template <int k_channels, typename TFeedbackLine, typename TCoefficients, typename TUIParams, typename TFilterParams, typename TDecoratorParams>
template <typename TBaseDecoratorParams>
FilterDecorator<k_channels, TFeedbackLine, TCoefficients, TUIParams, TFilterParams, TDecoratorParams>
::FilterDecorator(FilterDecorator<k_channels, TFeedbackLine, TCoefficients, TUIParams, TFilterParams, TBaseDecoratorParams> *fd, TDecoratorParams *p)
: FilterBase<k_channels, TFeedbackLine, TCoefficients, TUIParams, TDecoratorParams>(p), filter_ptr(fd->filter_ptr) { }

template <int k_channels, typename TFeedbackLine, typename TCoefficients, typename TUIParams, typename TFilterParams, typename TDecoratorParams>
void FilterDecorator<k_channels, TFeedbackLine, TCoefficients, TUIParams, TFilterParams, TDecoratorParams>::prepare_parameters(const TUIParams &params)
{
    this->filter_ptr->prepare_parameters(params);
}

template <int k_channels, typename TFeedbackLine, typename TCoefficients, typename TUIParams, typename TFilterParams, typename TDecoratorParams>
TCoefficients FilterDecorator<k_channels, TFeedbackLine, TCoefficients, TUIParams, TFilterParams, TDecoratorParams>::prepare_coefficients()
{
    return filter_ptr->prepare_coefficients();
}

template <int k_channels, typename TFeedbackLine, typename TCoefficients, typename TUIParams, typename TFilterParams, typename TDecoratorParams>
void FilterDecorator<k_channels, TFeedbackLine, TCoefficients, TUIParams, TFilterParams, TDecoratorParams>::process_frame(const TCoefficients &coeff, const float x[k_channels], float y[k_channels])
{
    // Handle channel iteration here to ensure virtual dispatch through decorator chain
    for (uint16_t channel = 0; channel < k_channels; channel++) {
        this->process_channel_frame(this->filter_ptr->state[channel], coeff, x[channel], y[channel]);
    }
    
}

template <int k_channels, typename TFeedbackLine, typename TCoefficients, typename TUIParams, typename TFilterParams, typename TDecoratorParams>
void FilterDecorator<k_channels, TFeedbackLine, TCoefficients, TUIParams, TFilterParams, TDecoratorParams>::process_channel_frame(TFeedbackLine &state, const TCoefficients &coeff, const float &x, float &y)
{
    this->filter_ptr->process_channel_frame(state, coeff, x, y);
}

template <int k_channels, typename TUIParams>
Butterworth<k_channels, TUIParams>::Butterworth(const uint32_t& p_sample_rate, ButterworthParameters *p_params)
: Filter<k_channels, FeedbackLine, NormalCoefficients, TUIParams, ButterworthParameters>(p_params), sample_rate(p_sample_rate) 
{ 
    // Initialize filter state to zero to prevent random behavior
    for (int i = 0; i < k_channels; i++) {
        this->state[i].x[0] = 0.0f;
        this->state[i].x[1] = 0.0f;
        this->state[i].y[0] = 0.0f;
        this->state[i].y[1] = 0.0f;
        this->state[i].fb = 0.0f;
    }
}

template <int k_channels, typename TUIParams>
void Butterworth<k_channels, TUIParams>::prepare_parameters(const TUIParams& params)
{
    // Direct logarithmic interpolation for smooth frequency scaling using standard math
    const float min_freq = 10.f;
    const float max_freq = 23000.f;  // Closer to Nyquist for "open" filter
    float log_freq = logf(min_freq) + params.p_cutoff * (logf(max_freq) - logf(min_freq));
    float raw_cutoff = expf(log_freq);
    this->params->cutoff = fminf(raw_cutoff, 0.48f * this->sample_rate);  // Allow closer to Nyquist

    // Resonance response
    this->params->res = params.p_resonance;

    // Base Q of 0.707 (Butterworth) plus resonance (clamped for stability)
    this->params->Q = M_SQRT1_2 + fminf(this->params->res, 10.f);
}

template <int k_channels, typename TUIParams>
void Butterworth<k_channels, TUIParams>::process_frame(const NormalCoefficients& coeff, 
                                              const float x[k_channels], 
                                              float y[k_channels])
{
    for (uint16_t channel = 0; channel < k_channels; channel++)
    {
        this->process_channel_frame(this->state[channel], coeff, x[channel], y[channel]);
    }
}

template <int k_channels, typename TUIParams>
void Butterworth<k_channels, TUIParams>::process_channel_frame(FeedbackLine& state,
                                                               const NormalCoefficients& coeff,
                                                               const float& x, 
                                                               float& y)
{
    this->filter(state, coeff, x, y);
    
    // Update feedback state
    state.x[1] = state.x[0];
    state.x[0] = x;
    state.y[1] = state.y[0];
    state.y[0] = y;
    state.fb = y;
}

template <int k_channels, typename TUIParams>
void Butterworth<k_channels, TUIParams>::filter(FeedbackLine &state, const NormalCoefficients &coeff, const float &x, float &y)
{
    // Direct Form I biquad - matches Audio EQ Cookbook exactly
    y = coeff.b0 * x + coeff.b1 * state.x[0] + coeff.b2 * state.x[1] 
        - coeff.a1 * state.y[0] - coeff.a2 * state.y[1];
}

template <int k_channels, typename TUIParams>
ButterworthHP<k_channels, TUIParams>::ButterworthHP(const uint32_t& p_sample_rate, ButterworthParameters *p_params)
: Butterworth<k_channels, TUIParams>(p_sample_rate, p_params) {}

template <int k_channels, typename TUIParams>
void ButterworthHP<k_channels, TUIParams>::process_channel_frame(FeedbackLine& state,
                                                                 const NormalCoefficients& coeff,
                                                                 const float& x, 
                                                                 float& y)
{
    // CRITICAL: Highpass filters require input feedback to work properly
    // This compensates for coefficient collapse at low frequencies
    const float fb_amount = this->params->res * 0.24f;
    float input = x - fb_amount * feedback_saturate(state.fb * 0.9f);
    
    Butterworth<k_channels, TUIParams>::process_channel_frame(state, coeff, input, y);
}

template <int k_channels, typename TUIParams>
NormalCoefficients ButterworthHP<k_channels, TUIParams>::prepare_coefficients()
{
    // Professional state variable filter approach with proper frequency scaling
    const float nyquist = this->sample_rate * 0.5f;
    const float freq = fminf(this->params->cutoff, nyquist * 0.99f);
    
    // Frequency warping compensation for accurate response
    const float wc = freq / nyquist;
    const float g = tanf(M_PI * wc * 0.5f);
    
    // Damping factor from Q with proper scaling
    const float k = 1.0f / this->params->Q;
    
    // State variable filter coefficients - corrected formulas (highpass)
    const float denom = 1.0f + g * (g + k);
    const float norm = 1.0f / denom;
    
    NormalCoefficients coeff = {
        .a1 = 2.0f * (g * g - 1.0f) * norm,
        .a2 = (1.0f - g * k + g * g) * norm,
        .b0 = norm,
        .b1 = -2.0f * norm,
        .b2 = norm
    };

    return coeff;
}

template <int k_channels, typename TUIParams>
ButterworthLP<k_channels, TUIParams>::ButterworthLP(const uint32_t& p_sample_rate, ButterworthParameters *p_params)
: Butterworth<k_channels, TUIParams>(p_sample_rate, p_params) {}

template <int k_channels, typename TUIParams>
NormalCoefficients ButterworthLP<k_channels, TUIParams>::prepare_coefficients()
{
    // Pre-warped bilinear transform - correct implementation
    const float w = tanf(M_PI * this->params->cutoff / this->sample_rate);
    const float w2 = w * w;
    const float cosw = (1.0f - w2) / (1.0f + w2);
    const float sinw = 2.0f * w / (1.0f + w2);
    const float alpha = sinw / (2.0f * this->params->Q);
    
    // Standard RBJ lowpass with pre-warped frequency
    const float norm = 1.0f / (1.0f + alpha);
    const float b0 = (1.0f - cosw) * 0.5f * norm;
    const float b1 = (1.0f - cosw) * norm;
    const float b2 = (1.0f - cosw) * 0.5f * norm;
    const float a1 = -2.0f * cosw * norm;
    const float a2 = (1.0f - alpha) * norm;
    
    NormalCoefficients coeff = {
        .a1 = a1,
        .a2 = a2,
        .b0 = b0,
        .b1 = b1,
        .b2 = b2
    };

    return coeff;
}

template <int k_channels, typename TUIParams, typename TFilterParams>
void Compensated<k_channels, TUIParams, TFilterParams>::process_channel_frame(FeedbackLine &state, 
                                                                  const NormalCoefficients &coeff,
                                                                  const float& x, 
                                                                  float& y)
{
    FilterDecorator<k_channels, FeedbackLine, NormalCoefficients, TUIParams, TFilterParams, CompensatedParameters>::process_channel_frame(state, coeff, x, y);
    y *= this->params->vol_comp;
}

template <int k_channels, typename TUIParams, typename TFilterParams>
void ResCompensated<k_channels, TUIParams, TFilterParams>::prepare_parameters(const TUIParams& params)
{
    // Call base class first to ensure filter parameters are set
    FilterDecorator<k_channels, FeedbackLine, NormalCoefficients, TUIParams, TFilterParams, CompensatedParameters>::prepare_parameters(params);
    
    // Use resonance directly from UI parameters instead of base filter state
    this->params->fb_amount = params.p_resonance * 0.24f;
    this->params->vol_comp = 1.f + (this->params->fb_amount);
}

template <int k_channels, typename TUIParams, typename TFilterParams>
void FreqCompensated<k_channels, TUIParams, TFilterParams>::prepare_parameters(const TUIParams& params)
{
    FilterDecorator<k_channels, FeedbackLine, NormalCoefficients, TUIParams, TFilterParams, CompensatedParameters>::prepare_parameters(params);

    // Reduced feedback with compensation for volume loss
    this->params->fb_amount = params.p_cutoff * 0.24f;
    
    // Volume compensation increases with resonance
    this->params->vol_comp = 1.f + (this->params->fb_amount);
}    

template <int k_channels, typename TUIParams, typename TFilterParams>
void Saturated<k_channels, TUIParams, TFilterParams>::prepare_parameters(const TUIParams& params)
{
    FilterDecorator<k_channels, FeedbackLine, NormalCoefficients, TUIParams, TFilterParams, SaturatedParameters>::prepare_parameters(params);

   // Subtle drive based on resonance - much gentler
    this->params->drive = 1.f + this->params->res * 0.3f * powf(params.p_cutoff, 2.f);
}

template <int k_channels, typename TUIParams, typename TFilterParams>
void Saturated<k_channels, TUIParams, TFilterParams>::process_channel_frame(FeedbackLine &state, const NormalCoefficients &coeff, const float &x, float &y)
{    
    FilterDecorator<k_channels, FeedbackLine, NormalCoefficients, TUIParams, TFilterParams, SaturatedParameters>::process_channel_frame(state, coeff, x, y);

    // Apply gentle saturation for musical character
    y = audio_saturate(y * this->params->drive) * (1.f / this->params->drive);
}
