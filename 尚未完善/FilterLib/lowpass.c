#include "lowpass.h"

int32_t lowpass_filter(lowpass_t* filter, int32_t input)
{
    /* 一阶 IIR: y[n] = y[n-1] + α · (x[n] − y[n-1]) */
    filter->last_output += filter->alpha * ((float)input - filter->last_output);
    return (int32_t)filter->last_output;
}

void lowpass_reset(lowpass_t* filter)
{
    filter->last_output = 0.0f;
}
