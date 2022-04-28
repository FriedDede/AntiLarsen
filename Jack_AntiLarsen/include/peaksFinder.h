//
// Created by andre on 4/26/22.
//

#ifndef JACK_ANTILARSEN_PEAKSFINDER_H
#define JACK_ANTILARSEN_PEAKSFINDER_H
#include "const.h"

class peaksFinder {
public:
    explicit peaksFinder(float *buffer);
    void findHowls(const float *, int*);
    void algoSelect(const bool[3]);
    int found_howls[N_PEAKS];
private:
    float *jack_buffer;
    float buffer[3][BUF_LENGTH];
    int current_buffer = 0;
    int found_peaks_num = 0;

    void phpr(const float *, int *);
    void pnpr(const float *, int *);
    void imsd(const float *, int *);
    void fftWrapper(const float *, float *);
    void updateBuffer();
    static void inline minHead(const float *, int *);
/*
 * Thresholds for howling frequencies detection (in dB)
 */
    const float phpr_threshold = 10;
    const float pnpr_threshold = 30;
    const float imsd_threshold = 1;

    bool run_phpr = true;
    bool run_pnpr = true;
    bool run_imsd = true;
};


#endif
