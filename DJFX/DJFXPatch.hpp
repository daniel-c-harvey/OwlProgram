#ifndef __DJFXPATCH_H__
#define __DJFXPATCH_H__

#include "Patch.h"
#include "ui.hpp"
#include "butterworth.hpp"

class DJFXPatch : public Patch {
    private:
        uint8_t k_channels;
        uint32_t k_samplerate;
        UserParameters* uparams;

        ButterworthParameters* hp_butterworth_params;
        CompensatedParameters* hp_compensated_params;
        SaturatedParameters* hp_saturated_params;

        ButterworthHP<2, FilterParameters>* hp_butterworth;
        ResCompensated<2, FilterParameters>* hp_compensated;
        Saturated<2, FilterParameters>* hp_saturated;
        FilterBase<2, FeedbackLine, NormalCoefficients, FilterParameters>* hp_filter;

        ButterworthParameters* lp_butterworth_params;
        CompensatedParameters* lp_compensated_params;
        SaturatedParameters* lp_saturated_params;

        ButterworthLP<2, FilterParameters>* lp_butterworth;
        FreqCompensated<2, FilterParameters>* lp_compensated;
        Saturated<2, FilterParameters>* lp_saturated;
        FilterBase<2, FeedbackLine, NormalCoefficients, FilterParameters>* lp_filter;

        float frame[2];

    public:
        DJFXPatch();
        ~DJFXPatch();
        void processAudio(AudioBuffer &buffer);
};

#endif // __DJFXPATCH_H__