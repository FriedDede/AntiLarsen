//
// Created by posta on 7/1/2022.
//

#include "../include/DSP.h"
#include <cmath>

DSP::DSP() {
    buf_in = nullptr;
    buf_out = nullptr;
    for (auto &f : this->bank.p_filter) {
        f = nullptr;
    }
    this->bank.active_filters = 0;
    this->bank.next_insert = 0;
    generateFilters(f_sampling,q_factor,f_min);
}

void DSP::add_filter_to_bank(int index) {
    if (index < N_PRE_FILTERS){
        if (this->bank.active_filters < MAX_ACTIVE_FILTERS){
            this->bank.p_filter[this->bank.active_filters] = &this->filters[index];
            this->bank.active_filters++;
        }
        if (this->bank.active_filters == MAX_ACTIVE_FILTERS){
            this->bank.p_filter[this->bank.next_insert] = &this->filters[index];
            this->bank.next_insert++;
            if (this->bank.next_insert == 9){
                this->bank.next_insert = 0;
            }
        }
    }
}

bool DSP::applyFilters() {
    // delayed feedforward (x_n), feedback(y_n) samples
    static float x_2 = 0.0f;
    static float x_1 = 0.0f;
    static float y_2 = 0.0f;
    static float y_1 = 0.0f;
    for (int i = 0; i < this->bank.active_filters; ++i) {
        if (this->bank.p_filter[i] != nullptr){
            for (int j = 0; j < BUF_LENGTH; ++j) {
                // IIR Filters Equation
                buf_out[j] = (this->bank.p_filter[i]->e * buf_in[j] +
                             this->bank.p_filter[i]->p * x_1 +
                             this->bank.p_filter[i]->d[0] * x_2 +
                             this->bank.p_filter[i]->d[1] * y_1 +
                             this->bank.p_filter[i]->d[2] * y_2);
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

void DSP::setInOutBuffers(float *in, float * out, int nframes_update) {
    if (nframes_update != this->nframes) {
        this->nframes = nframes_update;
    }
    buf_out = out;
    buf_in = in;
}

float *DSP::getBufOut() const {
    return buf_out;
}

void DSP::reset_bank() {
    bank.active_filters = 0;
    bank.next_insert = 0;
    for (auto &f: bank.p_filter){
        f = nullptr;
    }
}

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
void DSP::generateFilters(float f_sampling_in, float q_factor_in, float f_min_in) {
    float damp;
    float wo;
    float f;
    float gb = 1.0f/sqrtf(2.0f);
    float bw;
    float beta;
    float gain;

    this->f_sampling = f_sampling_in;
    this->q_factor = q_factor_in;
    this->f_step = (f_sampling_in/2 - f_min_in) / (float)N_PRE_FILTERS;
    this->f_min = f_min_in;

    f= f_min_in;

    damp = sqrtf(std::abs(1.0f - powf(gb, 2.0f)))/ gb;
    // FILTERS DESIGN TYPE 1 ------------- //

    for (auto &filter: this->filters) {

        wo = 2.0f * PI * (f) / (f_sampling_in);
        bw = wo / q_factor_in;
        beta = damp*tanf(bw/2.0f);
        gain = 1.0f/(1.0f + beta);
        // numerator - feed forward
        filter.e = 1.0f * gain;
        filter.p = -2.0f * cosf(wo) * gain;
        filter.d[0] = filter.e;
        // denominator - feed back
        filter.d[1] = filter.p;
        filter.d[2] = -(2.0f*gain-1.0f);
        f += f_step;
    }

    // FILTERS DESIGN TYPE 2 ------------- //
    /*
    float r = 0.99f;
    for (auto &filter: this->filters) {
        wo = 2.0f * PI * (f) / (f_sampling_in);
        filter.e = 1.0f;
        filter.p = -2*cosf(wo);
        filter.d[0] = filter.e;
        filter.d[1] = -2*r*cosf(wo);
        filter.d[2] = -r*r;
        f += f_step;
    }
     */
}
