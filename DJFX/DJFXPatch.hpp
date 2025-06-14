#ifndef __DJFXPATCH_H__
#define __DJFXPATCH_H__

#include "Patch.h"
#include "ui.hpp"
#include "butterworth.hpp"

class DJFXPatch : public Patch {
    private:
        uint8_t k_channels;
        uint32_t k_samplerate;
        uint16_t k_frames;
        UserParameters* uparams;

        ButterworthParameters* hp_butterworth_params;
        CompensatedParameters* hp_compensated_params;
        SaturatedParameters* hp_saturated_params;

        ButterworthHP<2, FilterParameters>* hp_butterworth;
        ResCompensated<2, FilterParameters, ButterworthParameters>* hp_compensated;
        Saturated<2, FilterParameters, ButterworthParameters>* hp_saturated;
        FilterBase<2, FeedbackLine, NormalCoefficients, FilterParameters, SaturatedParameters>* hp_filter;

        ButterworthParameters* lp_butterworth_params;
        CompensatedParameters* lp_compensated_params;

        ButterworthLP<2, FilterParameters>* lp_butterworth;
        FreqCompensated<2, FilterParameters, ButterworthParameters>* lp_compensated;
        FilterBase<2, FeedbackLine, NormalCoefficients, FilterParameters, CompensatedParameters>* lp_filter;

        float frame[2];

    public:
        DJFXPatch();
        ~DJFXPatch();
        void processAudio(AudioBuffer &buffer);
};

#endif // __DJFXPATCH_H__