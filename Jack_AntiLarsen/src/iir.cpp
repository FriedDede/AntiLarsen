//
// Created by andre on 4/25/22.
// filters class definition
//

#include "../include/iir.h"
#include <cmath>


/*
 * precomputed filter init:
 * Generates a filter (n = MAX_FILTERS) of second order IIR notch filter
 * params:
 * @ f_sampling = sampling frequency (in Hz)
 * @ gb = gain at center frequency
 * @ q_factor = quality factor, q = w0/bw, bw is the -3dB bandwidth
 * @ f_step = frequency resolution
 * @ f_min = lowest filtered frequency
 */
preFiltersBank::preFiltersBank(float f_sampling, float gb, float q_factor, float f_step, float f_min) {
    int i = 0;
    float damp;
    float wo;

    this->f_sampling = f_sampling;
    this->gb = gb;
    this->q_factor = q_factor;
    this->f_step = (f_sampling/2 - f_min)/MAX_FILTERS;
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

void activeFilters::add_filter_to_bank(int index,t_filter filters[]) {
    if (this->bank.active_filters < MAX_ACTIVE_FILTERS){
        this->bank.p_filter[this->bank.active_filters] = &filters[index];
    }
    if (this->bank.active_filters == MAX_ACTIVE_FILTERS){
        this->bank.p_filter[this->bank.next_insert] = &filters[index];
        this->bank.next_insert++;
        if (this->bank.next_insert == 10){
            this->bank.next_insert = 0;
        }
    }
}

bool activeFilters::apply(const float *buf_in, float *buf_out) {
    // delayed x, y samples
    static float x_2 = 0.0f;
    static float x_1 = 0.0f;
    static float y_2 = 0.0f;
    static float y_1 = 0.0f;
    for (int i = 0; i < this->bank.active_filters; ++i) {
        if (this->bank.p_filter[i] != nullptr){
            for (int j = 0; j < BUF_LENGTH; ++j) {
                // IIR Filters Equation
                buf_out[j] = this->bank.p_filter[i]->e * buf_in[j] +
                             this->bank.p_filter[i]->p * x_1 +
                             this->bank.p_filter[i]->d[0] * x_2 +
                             this->bank.p_filter[i]->d[1] * y_1 +
                             this->bank.p_filter[i]->d[2] * y_2;
                // shift delayed x, y samples
                x_2 = x_1;
                x_1 = buf_in[j];
                y_2 = y_1;
                y_1 = buf_out[j];
            }
        }
        else{
            return false;
        }
        x_2 = 0.0f;
        x_1 = 0.0f;
        y_2 = 0.0f;
        y_1 = 0.0f;
    }
    return true;
}

activeFilters::activeFilters() {
    for (auto &f : this->bank.p_filter) {
            f = nullptr;
    }
    this->bank.active_filters = 0;
    this->bank.next_insert = 0;
}
