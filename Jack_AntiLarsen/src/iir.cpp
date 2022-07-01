//
// Created by andre on 4/25/22.
// filters class definition
//

#include "../include/iir.h"
#include <cmath>
/*
 * precomputed filter init:
 * Generates a filter (n = N_PRE_FILTERS) of second order IIR notch filter
 * params:
 * @ f_sampling = sampling frequency (in Hz)
 * @ gb = gain at center frequency
 * @ q_factor = quality factor, q = w0/bw, bw is the -3dB bandwidth
 * @ f_min = lowest filtered frequency
 */
preFiltersBank::preFiltersBank(float f_sampling, float gb, float q_factor, float f_min) {
    int i = 0;
    float damp;
    float wo;

    this->f_sampling = f_sampling;
    this->gb = gb;
    this->q_factor = q_factor;
    this->f_step = (25000.0f - f_min)/(float)N_PRE_FILTERS;
    this->f_min = f_min;

    damp = sqrt(1 - pow(gb,2))/gb;
    for (auto &filter: this->filters) {
        wo = 2*PI*(f_step*i + f_min)/f_sampling;
        filter.e = 1/(1 + damp*tan(wo/(q_factor*2)));
        filter.p = cos(wo);
        filter.d[0] = filter.e;
        filter.d[1] = 2*filter.e*filter.p;
        filter.d[2] = (2*filter.e-1);
        i++;
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
