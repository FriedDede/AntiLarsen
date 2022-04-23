#include <stdio.h>
#include "iir.h"

int main() {
    t_br_coeffs filters[BR_MAX_COEFS];
    float f_sampling = 44100;
    float gb = -10;
    float q_factor = 10;
    float f_step = 10;
    short f_min  = 1000;

    br_iir_init(filters,f_sampling,gb,q_factor,f_step,f_min);
    printf("Hello, World!\n");
    return 0;
}
