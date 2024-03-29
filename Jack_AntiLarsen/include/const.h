//
// Created by andre on 4/26/22.
//

#ifndef JACK_ANTILARSEN_CONST_H
#define JACK_ANTILARSEN_CONST_H

#define BUF_LENGTH 1024
#define N_PRE_FILTERS 512
#define MAX_ACTIVE_FILTERS 10
#define PI 3.1415926f
#define N_PEAKS 10
#define FSTEP 46.875f //Hz
#define INT_TIME 21.3f //ms
/*
 * r2c fftw output format:
 * https://www.fftw.org/fftw3_doc/The-1d-Real_002ddata-DFT.html
 */
#define FFTOUT_BUF_LENGTH BUF_LENGTH/2+1
#endif
