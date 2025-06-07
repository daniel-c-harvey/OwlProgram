#pragma once

struct FilterParameters
{
    float p_cutoff;
    float p_resonance;
};

class UserParameters
{
    public:
        void setHP(float);
        void setLP(float);

        FilterParameters getHPParams();
        FilterParameters getLPParams();

    protected:
        FilterParameters hp_params;
        FilterParameters lp_params;
};

