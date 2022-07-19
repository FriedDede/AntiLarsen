//
// Created by posta on 7/1/2022.
//

#include "../include/DSP.h"
#include <cstring>
#include "jack/types.h"
#include "malloc.h"

DSP::DSP() {
    buf_in = nullptr;
    buf_out = nullptr;
    for (auto &f : this->bank.p_filter) {
        f = nullptr;
    }
    this->bank.active_filters = 0;
    this->bank.next_insert = 0;
}

void DSP::add_filter_to_bank(int index, t_filter filters[]) {
    if (index < N_PRE_FILTERS){
        if (this->bank.active_filters < MAX_ACTIVE_FILTERS){
            this->bank.p_filter[this->bank.active_filters] = &filters[index];
            this->bank.active_filters++;
        }
        if (this->bank.active_filters == MAX_ACTIVE_FILTERS){
            this->bank.p_filter[this->bank.next_insert] = &filters[index];
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
                buf_out[j] = 0.5f*(this->bank.p_filter[i]->e * buf_in[j] +
                             this->bank.p_filter[i]->p * x_1 +
                             this->bank.p_filter[i]->d[0] * x_2 -
                             this->bank.p_filter[i]->d[1] * y_1 -
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
