//
// Created by andre on 4/25/22.
// br filter class definition
//

#include "iir.h"
/*
 * filters bank init:
 * Generates a bank (n = MAX_FILTERS) of second order IIR notch filter
 * params:
 * @ f_sampling = sampling frequency (in Hz)
 * @ gb = gain at center frequency
 * @ q_factor = quality factor, q = w0/bw, bw is the -3dB bandwidth
 * @ f_step = frequency resolution
 * @ f_min = lowest filtered frequency
 */
filter_bank::filter_bank(float f_sampling, float gb, float q_factor, float f_step, float f_min) {
    int i = 0;
    float damp;
    float wo;

    this->f_sampling = f_sampling;
    this->gb = gb;
    this->q_factor = q_factor;
    this->f_step = (f_sampling/2 - f_min)/MAX_FILTERS;
    this->f_min = f_min;

    damp = sqrt(1 - pow(this->gb,2))/this->gb;
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

void filter_bank::br_iir_setup(t_br_coeffs *, struct br_filter *H, int index) {
    H->e = br_coeff_arr[ind].e;
    H->p = br_coeff_arr[ind].p;
    H->d[0] = br_coeff_arr[ind].d[0];
    H->d[1] = br_coeff_arr[ind].d[1];
    H->d[2] = br_coeff_arr[ind].d[2];
}

float filter_bank::br_iir_filter(float yin, struct br_filter *H) {
    float yout;

    H->x[0] = H->x[1];
    H->x[1] = H->x[2];
    H->x[2] = yin;

    H->y[0] = H->y[1];
    H->y[1] = H->y[2];

    H->y[2] = H->d[0]*H->x[2] - H->d[1]*H->x[1] + H->d[0]*H->x[0] + H->d[1]*H->y[1] - H->d[2]*H->y[0];

    yout = H->y[2];
    return yout;
    return 0;
}

