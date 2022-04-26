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
 * active filters' pointer bank
 */
typedef struct bank{
    t_filter *bank[MAX_ACTIVE_FILTERS];
    int active_filters = 0;
    int next_insert = 0;
}t_bank;
/*
 * Notch filter's buffer
 */
typedef struct iir_filter_buffer {
    float e;
    float p;
    float d[3];
    float x[3];
    float y[3];
} t_notch_filter;
/*
 * Precomputed filters' bank
 */
class preFiltersBank{
    preFiltersBank(float , float , float , float , float );

public:
    t_filter filters[MAX_FILTERS]{};

private:
    float f_sampling = 44100;
    float gb = -10;
    float q_factor = 30;
    float f_step;
    float f_min  = 100;
};
/*
 * Active filters' bank
 * Stores pointers to active filters
 * Apply filters to audio frame
 */
class activeFilters{
public:
    void apply(const float *src, float *dest);
    void add_filter_to_bank(int index,t_filter*);
    t_bank bank;
};

#endif
