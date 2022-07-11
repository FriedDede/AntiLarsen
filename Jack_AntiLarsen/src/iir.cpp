//
// Created by andre on 4/25/22.
// filters class definition
//

#include "../include/iir.h"
#include <cmath>
#include <complex>
/*
 * precomputed filter init:
 * Generates a filter (n = N_PRE_FILTERS) of second order IIR notch filter
 * params:
 * @ f_sampling_in = sampling frequency (in Hz)
 * @ gb_in = gain at center frequency
 * @ q_factor_in = quality factor, q = w0/bw, bw is the -3dB bandwidth
 * @ f_min_in = lowest filtered frequency
 * TODO: verify the math is correct (studia capra)
 */
preFiltersBank::preFiltersBank(float f_sampling_in, float gb_in, float q_factor_in, float f_min_in) {
    float damp;
    float wo;
    float f;

    this->f_sampling = f_sampling_in;
    this->gb = gb_in;
    this->q_factor = q_factor_in;
    this->f_step = (f_sampling_in/2 - f_min_in) / (float)N_PRE_FILTERS;
    this->f_min = f_min_in;

    f= f_min_in;

    damp = sqrtf(std::abs(1.0f - powf(gb_in, 2.0f)))/ gb_in;
    for (auto &filter: this->filters) {
        wo = 2.0f * PI * (f) / (f_sampling_in/2);
        filter.e = 1.0f/(1.0f + damp*tanf(wo/(q_factor_in * 2.0f)));
        filter.p = cosf(wo);
        filter.d[0] = filter.e;
        filter.d[1] = 2.0f*filter.e*filter.p;
        filter.d[2] = (2.0f*filter.e-1.0f);
        f += f_step;
    }
}

void preFiltersBank::setFSampling(float new_f_sampling) {
    this->f_sampling = new_f_sampling;
}

void preFiltersBank::setGb(float new_gb) {
    this->gb = new_gb;
}

void preFiltersBank::setQFactor(float new_q_factor) {
    this->q_factor = new_q_factor;
}
