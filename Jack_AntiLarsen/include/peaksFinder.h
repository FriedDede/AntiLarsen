//
// Created by andre on 4/26/22.
//

#ifndef JACK_ANTILARSEN_PEAKSFINDER_H
#define JACK_ANTILARSEN_PEAKSFINDER_H
#include "const.h"

class peaksFinder {
public:
    peaksFinder(float *buffer);
    void updateBuffer();
    void fftWrapper(const float *, float *);
    void findHowls();
    void phpr();
    void pnpr();
    void imsd();


    int found_howls[PEAKS];
private:
    float *jack_buffer;
    float buffer[3][BUF_LENGTH]{};
    int current_buffer = 0;
    int found_peaks_num = 0;
/*
 * Thresholds for howling frequencies detection (in dB)
 */
    const float phpr_threshold = 10;
    const float pnpr_threshold = 30;
    const float imsd_threshold = 1;
};


#endif
