//
// Created by andre on 4/22/22.
//

#ifndef ANTILARSEN_IIR_H
#define ANTILARSEN_IIR_H
#include <cmath>

#define MAX_FILTERS 2048
#define PI 3.1415926

/*This initialization function will create the
notch filter coefficients array, you have to specify:
fsfilt = Sampling Frequency
gb     = Gain at cut frequencies
Q      = Q factor, Higher Q gives narrower band
fstep  = Frequency step to increase center frequencies
in the array
fmin   = Minimum frequency for the range of center   frequencies
*/

/*
Notch filter coefficients object
*/
typedef struct br_coeffs {
    float e;
    float p;
    float d[3];
} t_br_coeffs;

/*
Notch filter object
*/
typedef struct br_filter {
    float e;
    float p;
    float d[3];
    float x[3];
    float y[3];
} t_notch_filter;

class filter_bank{
    filter_bank(float , float , float , float , float );

public:
    void br_iir_setup(t_br_coeffs*, struct br_filter * H,int index);
    float br_iir_filter(float yin,struct br_filter * H);
    t_br_coeffs filters[MAX_FILTERS];
    int in_use[10];
private:
    float f_sampling = 44100;
    float gb = -10;
    float q_factor = 30;
    float f_step;
    float f_min  = 100;

};


#endif