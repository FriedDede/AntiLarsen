//
// Created by andre on 4/22/22.
//

#ifndef ANTILARSEN_IIR_H
#define ANTILARSEN_IIR_H
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
 * Precomputed filters bank
 */
class preFiltersBank{

public:
    t_filter filters[MAX_FILTERS]{};
    preFiltersBank(float , float , float , float );
    void setFSampling(float new_f_sampling);
    void setGb(float new_gb);
    void setQFactor(float new_q_factor);

private:
    // Default values
    float f_sampling = 44100;
    float gb = -10;
    float q_factor = 30;
    float f_step;
    float f_min  = 0;
};
/*
 * Active filters class
 * Stores pointers to active p_filter
 * Apply p_filter to audio frame
 */
class activeFilters{
public:
    activeFilters();
    bool applyFilters();
    void add_filter_to_bank(int index,t_filter*);
    t_bank bank;
    void setIn(float *in);
    void setOut(float *out);
private:
    float *buf_in;
    float *buf_out;

};

#endif
