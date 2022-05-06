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
    explicit peaksFinder(const float *input, const bool *settings);
    void run();
    void setEnableAlgo(const bool *);

    void setPhprThreshold(float phprThreshold);
    void setPnprThreshold(float pnprThreshold);
    void setImsdThreshold(float imsdThreshold);
    float getPhprThreshold() const;
    float getPnprThreshold() const;
    float getImsdThreshold() const;
    bool isRunPhpr() const;
    bool isRunPnpr() const;
    bool isRunImsd() const;
    int found_howls[N_PEAKS];
    virtual ~peaksFinder();
private:
    fftw_plan ft_plan;
    std::complex<float> *ft_in;
    std::complex<float> *ft_out;
    const float *jack_buffer;
public:
    void setJackBuffer(const float *jackBuffer);

private:
    float *buffers[3];
    int current_buffer = 0;

    bool phpr(const float *);
    bool pnpr(const float *);
    bool imsd(const float *);
    void fftWrapper(const float *, float *);
    void updateBuffer();
    static void inline minHead(const float*, int *);
/*
 * Thresholds for howling frequencies detection (in dB)
 */
private:
    float phpr_threshold = 10.0f;
    float pnpr_threshold = 30.0f;
    float imsd_threshold = 1.0f;

    bool run_phpr = true;
    bool run_pnpr = true;
    bool run_imsd = true;
};


#endif
