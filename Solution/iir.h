//
// Created by andre on 4/22/22.
//

#ifndef ANTILARSEN_IIR_H
#define ANTILARSEN_IIR_H
#include <math.h>

#define BR_MAX_COEFS 2000
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
struct br_filter {
    float e;
    float p;
    float d[3];
    float x[3];
    float y[3];
};

extern void br_iir_init(t_br_coeffs*, float fsfilt,float gb,float Q,float fstep, short fmin);
extern void br_iir_setup(t_br_coeffs*, struct br_filter * H,int index);
extern float br_iir_filter(float yin,struct br_filter * H);

#endif
