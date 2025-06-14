#include "DJFXPatch.hpp"

DJFXPatch::DJFXPatch()
{
    k_channels = getNumberOfChannels();
    k_samplerate = getSampleRate();
    k_frames = getBlockSize();

    uparams = new UserParameters();
    hp_butterworth_params = new ButterworthParameters();
    hp_compensated_params = new CompensatedParameters();
    hp_saturated_params = new SaturatedParameters();

    hp_butterworth = new ButterworthHP<2, FilterParameters>(k_samplerate, hp_butterworth_params);
    hp_compensated = new ResCompensated<2, FilterParameters, ButterworthParameters>(hp_butterworth, hp_compensated_params);
    hp_saturated = new Saturated<2, FilterParameters, ButterworthParameters>(hp_compensated, hp_saturated_params);
    hp_filter = hp_saturated;

    lp_butterworth_params = new ButterworthParameters();
    lp_compensated_params = new CompensatedParameters();

    lp_butterworth = new ButterworthLP<2, FilterParameters>(k_samplerate, lp_butterworth_params);
    lp_compensated = new FreqCompensated<2, FilterParameters, ButterworthParameters>(lp_butterworth, lp_compensated_params);
    lp_filter = lp_compensated;

    registerParameter(PARAMETER_A, "HighPass");
    registerParameter(PARAMETER_B, "LowPass");
    setParameterValue(PARAMETER_A, 0.0);
    setParameterValue(PARAMETER_B, 0.0);
}

DJFXPatch::~DJFXPatch()
{
    delete uparams;
    delete hp_butterworth;
    delete hp_compensated;
    delete hp_saturated;
    delete hp_butterworth_params;
    delete hp_compensated_params;
    delete hp_saturated_params;
    delete lp_butterworth;
    delete lp_compensated;
    delete lp_butterworth_params;
    delete lp_compensated_params;
}

void DJFXPatch::processAudio(AudioBuffer &buffer)
{
    uparams->setHP(getParameterValue(PARAMETER_A));
    uparams->setLP(getParameterValue(PARAMETER_B));

    hp_filter->prepare_parameters(uparams->getHPParams());
    lp_filter->prepare_parameters(uparams->getLPParams());

    NormalCoefficients hp_coeff = hp_filter->prepare_coefficients();
    NormalCoefficients lp_coeff = lp_filter->prepare_coefficients();

    FloatArray left = buffer.getSamples(LEFT_CHANNEL);
    FloatArray right = buffer.getSamples(RIGHT_CHANNEL);

    for (uint16_t frame_index = 0; frame_index < k_frames; frame_index++)
    {
        frame[0] = left[frame_index];
        frame[1] = right[frame_index];

        hp_filter->process_frame(hp_coeff, frame, frame);
        lp_filter->process_frame(lp_coeff, frame, frame);

        left[frame_index] = frame[0];
        right[frame_index] = frame[1];
    }
}
