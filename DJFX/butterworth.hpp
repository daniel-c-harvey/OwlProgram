#pragma once

#include "util.hpp"

template <int k_channels, typename TFeedbackLine, typename TCoefficients, typename TUIParams>
class FilterBase
{
    public:        
        /// @brief Prepare the filter channels to process all frames in this block
        virtual void prepare_parameters(const TUIParams& params) = 0;

        /// @brief Prepare the filter channels to process all frames in this block
        virtual TCoefficients prepare_coefficients() = 0;
                
        /// @brief process the current frame samples for all channels
        /// @param x inputs samples
        /// @param y output samples
        virtual void process_frame(const TCoefficients& coeff, const float x[k_channels], float y[k_channels]) = 0;
        
        /// @brief process the current frame sample for given channel
        /// @param x inputs sample
        /// @param y output sample
        virtual void process_channel_frame(TFeedbackLine& state, const TCoefficients& coeff, const float& x, float& y) = 0;

        TFeedbackLine state[k_channels];
};

template <int k_channels, typename TFeedbackLine, typename TCoefficients, typename TUIParams, typename TFilterParams>
class Filter : public FilterBase<k_channels, TFeedbackLine, TCoefficients, TUIParams>
{
    public:
        explicit Filter(TFilterParams *p);

    protected:
        TFilterParams* params;
};

template <int k_channels, typename TFeedbackLine, typename TCoefficients, typename TUIParams, typename TFilterParams>
class FilterDecorator : public Filter<k_channels, TFeedbackLine, TCoefficients, TUIParams, TFilterParams>
{
    public:
        FilterDecorator(FilterBase<k_channels, TFeedbackLine, TCoefficients, TUIParams> *f, TFilterParams *p);

        void prepare_parameters(const TUIParams& params) override;

        TCoefficients prepare_coefficients() override;

        void process_frame(const TCoefficients& coeff, const float x[k_channels], float y[k_channels]) override;

    protected:
        FilterBase<k_channels, TFeedbackLine, TCoefficients, TUIParams> *filter_ptr;

        void process_channel_frame(TFeedbackLine& state, const TCoefficients& coeff, const float& x, float& y) override;
};

struct ButterworthParameters 
{
    float cutoff;
    float res;
    float Q;
};

typedef struct {
    float a1;
    float a2;
    float b0;
    float b1;
    float b2;
} NormalCoefficients;

typedef struct {
    float x[2];  // Previous inputs
    float y[2];  // Previous outputs
    float fb;  // Feedback value for resonance
} FeedbackLine;

template <int k_channels, typename TUIParams>
class Butterworth : public Filter<k_channels, FeedbackLine, NormalCoefficients, TUIParams, ButterworthParameters>
{
    public:
        Butterworth(const uint32_t& sample_rate, ButterworthParameters *params);

        void prepare_parameters(const TUIParams& params) override;
        
        void process_frame(const NormalCoefficients& coeff, 
                           const float x[k_channels], 
                           float y[k_channels]) override;
    protected:
        uint32_t sample_rate;

        void process_channel_frame(FeedbackLine& state, 
                                   const NormalCoefficients& coeff, 
                                   const float& x, 
                                   float& y) override;

        virtual void filter(FeedbackLine& state, 
                            const NormalCoefficients& coeff, 
                            const float& x, 
                            float& y);
};

template <int k_channels, typename TUIParams>
class ButterworthHP : public Butterworth<k_channels, TUIParams>
{
    public:
        ButterworthHP(const uint32_t& sample_rate, ButterworthParameters *params);

        NormalCoefficients prepare_coefficients() override;
};

template <int k_channels, typename TUIParams>
class ButterworthLP : public Butterworth<k_channels, TUIParams>
{
    public:
        ButterworthLP(const uint32_t& sample_rate, ButterworthParameters *params);

        NormalCoefficients prepare_coefficients() override;
};

struct CompensatedParameters : public ButterworthParameters
{
    float vol_comp; // Compensate for resonance-induced volume loss
    float fb_amount; // feedback
};

template <int k_channels, typename TUIParams>
class Compensated : public FilterDecorator<k_channels, FeedbackLine, NormalCoefficients, TUIParams, CompensatedParameters> 
{
    public:
        Compensated(FilterBase<k_channels, FeedbackLine, NormalCoefficients, TUIParams> *f, CompensatedParameters *p)
        : FilterDecorator<k_channels, FeedbackLine, NormalCoefficients, TUIParams, CompensatedParameters>(f, p) {}

        void prepare_parameters(const TUIParams& params) override = 0;

    protected:
        void process_channel_frame(FeedbackLine& state,
                                   const NormalCoefficients& coeff,
                                   const float& x, 
                                   float& y) override;
};

template <int k_channels, typename TUIParams>
class ResCompensated : public Compensated<k_channels, TUIParams> 
{
    public:
        ResCompensated(FilterBase<k_channels, FeedbackLine, NormalCoefficients, TUIParams> *f, CompensatedParameters *p)
        : Compensated<k_channels, TUIParams>(f, p) {}

        void prepare_parameters(const TUIParams& params) override;
};

template <int k_channels, typename TUIParams>
class FreqCompensated : public Compensated<k_channels, TUIParams> 
{
    public:
        FreqCompensated(FilterBase<k_channels, FeedbackLine, NormalCoefficients, TUIParams> *f, CompensatedParameters *p)
        : Compensated<k_channels, TUIParams>(f, p) {}

        void prepare_parameters(const TUIParams& params) override;
};

struct SaturatedParameters : public CompensatedParameters
{
    float drive;
};

template <int k_channels, typename TUIParams>
class Saturated : public FilterDecorator<k_channels, FeedbackLine, NormalCoefficients, TUIParams, SaturatedParameters> 
{
    public:
        Saturated(FilterBase<k_channels, FeedbackLine, NormalCoefficients, TUIParams> *f, SaturatedParameters *p)
        : FilterDecorator<k_channels, FeedbackLine, NormalCoefficients, TUIParams, SaturatedParameters>(f, p) {}

        void prepare_parameters(const TUIParams& params) override;

    protected:
        void process_channel_frame(FeedbackLine& state,
                                   const NormalCoefficients& coeff,
                                   const float& x, 
                                   float& y) override;
};

#include "butterworth.tpp"