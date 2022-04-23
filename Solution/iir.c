//
// Created by andre on 4/22/22.
//

#include "iir.h"

void br_iir_init(t_br_coeffs* br_coeff_arr, float fsfilt,float gb,float Q,float fstep, short fmin) {
    int i;
    float damp;
    float wo;

    damp = sqrt(1 - pow(gb,2))/gb;

    for (i=0;i<BR_MAX_COEFS;i++) {
        wo = 2*PI*(fstep*i + fmin)/fsfilt;
        br_coeff_arr[i].e = 1/(1 + damp*tan(wo/(Q*2)));
        br_coeff_arr[i].p = cos(wo);
        br_coeff_arr[i].d[0] = br_coeff_arr[i].e;
        br_coeff_arr[i].d[1] = 2*br_coeff_arr[i].e*br_coeff_arr[i].p;
        br_coeff_arr[i].d[2] = (2*br_coeff_arr[i].e-1);
    }
}

/*This function loads a given set of notch filter coefficients acording to a center frequency index
into a notch filter object
H = filter object
ind = index of the array mapped to a center frequency
*/
void br_iir_setup(t_br_coeffs* br_coeff_arr, struct br_filter * H,int ind) {
    H->e = br_coeff_arr[ind].e;
    H->p = br_coeff_arr[ind].p;
    H->d[0] = br_coeff_arr[ind].d[0];
    H->d[1] = br_coeff_arr[ind].d[1];
    H->d[2] = br_coeff_arr[ind].d[2];
}

/*
This function does the actual notch filter processing
yin = input sample
H   = filter object
*/
float br_iir_filter(float yin,struct br_filter * H) {
    float yout;

    H->x[0] = H->x[1];
    H->x[1] = H->x[2];
    H->x[2] = yin;

    H->y[0] = H->y[1];
    H->y[1] = H->y[2];

    H->y[2] = H->d[0]*H->x[2] - H->d[1]*H->x[1] + H->d[0]*H->x[0] + H->d[1]*H->y[1] - H->d[2]*H->y[0];

    yout = H->y[2];
    return yout;
}