//
// Created by posta on 7/1/2022.
//
#ifndef JACK_ANTILARSEN_ACTIVEFILTERS_H
#define JACK_ANTILARSEN_ACTIVEFILTERS_H
#include "const.h"
/*
 * Iir Notch filter's coefficients
 */
typedef struct iir_filter {
    float e;
    float p;
    float d[3];
} t_filter;
/*
 * active filters bank
 */
typedef struct bank{
    t_filter *p_filter[MAX_ACTIVE_FILTERS];
    int active_filters = 0;
    int next_insert = 0;
}t_bank;
/*
 * Active filters class
 * Stores pointers to active p_filter
 * Apply p_filter to audio frame
 */
class DSP{
public:
    DSP();
public:
    t_filter filters[N_PRE_FILTERS];
    void generateFilters(float , float , float );
    bool applyFilters();
    void reset_bank();
    void add_filter_to_bank(int index);
    void setInOutBuffers(float *in, float* out, int nframes);
    float *getBufOut() const;
private:
    float *buf_in;
    float *buf_out;
    int nframes = 0;
    t_bank bank;

    // Default values
    float f_sampling = 48000.0f;
    float q_factor = 35.0f;
    float f_step;
    float f_min  = 0.0f;
};

#endif
