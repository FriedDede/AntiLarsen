//
// Created by andre on 4/26/22.
//

#ifndef JACK_ANTILARSEN_PEAKSFINDER_H
#define JACK_ANTILARSEN_PEAKSFINDER_H
#include "const.h"
#include <fftw3.h>
#include <complex>

class peaksFinder {
public:
    explicit peaksFinder(const float *jack_buffer, const bool *settings);
    void run();
    void algoSelect(const bool[3]);

    virtual ~peaksFinder();
private:
    fftw_plan ft_plan;
    std::complex<float> *ft_in;
    std::complex<float> *ft_out;
    int *found_howls;
    const float *jack_buffer;
    float *buffers[3];
    int current_buffer = 0;

    void phpr(const float *, int *);
    void pnpr(const float *, int *);
    void imsd(const float *, int *);
    void fftWrapper(const float *, float *);
    void updateBuffer();
    static void inline minHead(const float*, int *);
/*
 * Thresholds for howling frequencies detection (in dB)
 */
    const float phpr_threshold = 10.0f;
    const float pnpr_threshold = 30.0f;
    const float imsd_threshold = 1.0f;

    bool run_phpr = true;
    bool run_pnpr = true;
    bool run_imsd = true;
};


#endif
