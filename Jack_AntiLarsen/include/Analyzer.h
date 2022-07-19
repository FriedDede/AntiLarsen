//
// Created by andre on 4/26/22.
//

#ifndef JACK_ANTILARSEN_ANALYZER_H
#define JACK_ANTILARSEN_ANALYZER_H
#include "const.h"
#include <fftw3.h>
#include <complex>

class Analyzer {
public:
    explicit Analyzer(const bool *settings);
    void analyzeBuffer(const float *jackBuffer);
    void setInputBuffer(const float *jackBuffer);
    float getPhprThreshold() const;
    float getPnprThreshold() const;
    float getImsdThreshold() const;
    bool isRunPhpr() const;
    bool isRunPnpr() const;
    bool isRunImsd() const;
    int found_howls[N_PEAKS];
    virtual ~Analyzer();

    std::complex<float> *getFtOut() const;
    float* getOutBuffer() const;

private:
    bool phpr(const float *);
    bool pnpr(const float *);
    bool imsd();
    void fftWrapper(const float *);
    static void inline minHead(const float*, int *);
    void blackman_win(int);

    fftwf_plan ft_plan;
    float* ft_in;
    std::complex<float>* ft_out;
    const float* jack_buffer;
    float* buffers[3];
    float blackman[BUF_LENGTH];
/*
 * Thresholds for howling frequencies detection (in dB)
 */
    static constexpr float phpr_threshold = 10.0f;
    static constexpr float pnpr_threshold = 25.5f;
    static constexpr float imsd_threshold = 1.0f;

    bool run_phpr = true;
    bool run_pnpr = true;
    bool run_imsd = true;
};


#endif
