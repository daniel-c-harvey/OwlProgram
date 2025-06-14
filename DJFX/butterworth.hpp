#pragma once

#include "util.hpp"

template <int k_channels, typename TFeedbackLine, typename TCoefficients, typename TUIParams, typename TFilterParams>
class FilterBase
{
    public:        
        FilterBase(TFilterParams *p);
        
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

        TFilterParams* params;
};

template <int k_channels, typename TFeedbackLine, typename TCoefficients, typename TUIParams, typename TFilterParams>
class Filter : public FilterBase<k_channels, TFeedbackLine, TCoefficients, TUIParams, TFilterParams>
{
    public:  
        Filter(TFilterParams *p) 
        : FilterBase<k_channels, TFeedbackLine, TCoefficients, TUIParams, TFilterParams>(p) {}

        TFeedbackLine state[k_channels];
};

template <int k_channels, typename TFeedbackLine, typename TCoefficients, typename TUIParams, typename TFilterParams, typename TDecoratorParams>
class FilterDecorator : public FilterBase<k_channels, TFeedbackLine, TCoefficients, TUIParams, TDecoratorParams>
{
    public:
        FilterDecorator(Filter<k_channels, TFeedbackLine, TCoefficients, TUIParams, TFilterParams> *f, TDecoratorParams *p);
        
        template <typename TBaseDecoratorParams>
        FilterDecorator(FilterDecorator<k_channels, TFeedbackLine, TCoefficients, TUIParams, TFilterParams, TBaseDecoratorParams> *f, TDecoratorParams *p);

        void prepare_parameters(const TUIParams& params) override;

        TCoefficients prepare_coefficients() override;

        void process_frame(const TCoefficients& coeff, const float x[k_channels], float y[k_channels]) override;

    protected:
        template <int, typename, typename, typename, typename, typename> friend class FilterDecorator;
        Filter<k_channels, TFeedbackLine, TCoefficients, TUIParams, TFilterParams> *filter_ptr;

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

        void filter(FeedbackLine& state, 
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
        
    protected:
        void process_channel_frame(FeedbackLine& state, 
                                   const NormalCoefficients& coeff, 
                                   const float& x, 
                                   float& y) override;
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

template <int k_channels, typename TUIParams, typename TFilterParams = ButterworthParameters>
class Compensated : public FilterDecorator<k_channels, FeedbackLine, NormalCoefficients, TUIParams, TFilterParams, CompensatedParameters> 
{
    public:
        Compensated(Filter<k_channels, FeedbackLine, NormalCoefficients, TUIParams, TFilterParams> *f, CompensatedParameters *p)
        : FilterDecorator<k_channels, FeedbackLine, NormalCoefficients, TUIParams, TFilterParams, CompensatedParameters>(f, p) {}

        Compensated(FilterDecorator<k_channels, FeedbackLine, NormalCoefficients, TUIParams, TFilterParams, CompensatedParameters> *f, CompensatedParameters *p)
        : FilterDecorator<k_channels, FeedbackLine, NormalCoefficients, TUIParams, TFilterParams, CompensatedParameters>(f, p) {}

        void prepare_parameters(const TUIParams& params) override = 0;

    protected:
        void process_channel_frame(FeedbackLine& state,
                                   const NormalCoefficients& coeff,
                                   const float& x, 
                                   float& y) override;
};

template <int k_channels, typename TUIParams, typename TFilterParams = ButterworthParameters>
class ResCompensated : public Compensated<k_channels, TUIParams, TFilterParams> 
{
    public:
        ResCompensated(Filter<k_channels, FeedbackLine, NormalCoefficients, TUIParams, TFilterParams> *f, CompensatedParameters *p)
        : Compensated<k_channels, TUIParams, TFilterParams>(f, p) {}

        ResCompensated(FilterDecorator<k_channels, FeedbackLine, NormalCoefficients, TUIParams, TFilterParams, CompensatedParameters> *f, CompensatedParameters *p)
        : Compensated<k_channels, TUIParams, TFilterParams>(f, p) {}

        void prepare_parameters(const TUIParams& params) override;
};

template <int k_channels, typename TUIParams, typename TFilterParams = ButterworthParameters>
class FreqCompensated : public Compensated<k_channels, TUIParams, TFilterParams> 
{
    public:
        FreqCompensated(Filter<k_channels, FeedbackLine, NormalCoefficients, TUIParams, TFilterParams> *f, CompensatedParameters *p)
        : Compensated<k_channels, TUIParams, TFilterParams>(f, p) {}

        FreqCompensated(FilterDecorator<k_channels, FeedbackLine, NormalCoefficients, TUIParams, TFilterParams, CompensatedParameters> *f, CompensatedParameters *p)
        : Compensated<k_channels, TUIParams, TFilterParams>(f, p) {}

        void prepare_parameters(const TUIParams& params) override;
};

struct SaturatedParameters : public ButterworthParameters
{
    float drive;
};

template <int k_channels, typename TUIParams, typename TBaseFilterParams = ButterworthParameters>
class Saturated : public FilterDecorator<k_channels, FeedbackLine, NormalCoefficients, TUIParams, TBaseFilterParams, SaturatedParameters> 
{
    public:
        Saturated(Filter<k_channels, FeedbackLine, NormalCoefficients, TUIParams, TBaseFilterParams> *f, SaturatedParameters *p)
        : FilterDecorator<k_channels, FeedbackLine, NormalCoefficients, TUIParams, TBaseFilterParams, SaturatedParameters>(f, p) {}

        template <typename TBaseDecoratorParams>
        Saturated(FilterDecorator<k_channels, FeedbackLine, NormalCoefficients, TUIParams, TBaseFilterParams, TBaseDecoratorParams> *f, SaturatedParameters *p)
        : FilterDecorator<k_channels, FeedbackLine, NormalCoefficients, TUIParams, TBaseFilterParams, SaturatedParameters>(f, p) {}

        void prepare_parameters(const TUIParams& params) override;

    protected:
        void process_channel_frame(FeedbackLine& state,
                                   const NormalCoefficients& coeff,
                                   const float& x, 
                                   float& y) override; 
};

#include "butterworth.tpp"