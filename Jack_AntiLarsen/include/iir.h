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
 * Precomputed filters bank
 */
class preFiltersBank{

public:
    t_filter filters[N_PRE_FILTERS];
    preFiltersBank(float , float , float , float );
    void setFSampling(float new_f_sampling);
    void setGb(float new_gb);
    void setQFactor(float new_q_factor);
private:
    // Default values
    float f_sampling = 48000;
    float gb = -10;
    float q_factor = 30;
    float f_step;
    float f_min  = 0;
};
#endif
