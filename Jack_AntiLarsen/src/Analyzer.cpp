//
// Created by andre on 4/26/22.
//

#include "../include/Analyzer.h"
#include "../include/fastmath.h"
#include <cmath>
#include <complex>
#include <fftw3.h>
#include <iostream>

Analyzer::Analyzer(const bool settings[3]) {
    // locate jack audio buffer
    this->jack_buffer = nullptr;
    /*
     * todo: test fftw_complex *fftw_malloc() for SSE/AVX speedup
     */
    this->ft_in = (std::complex<float> *) calloc(BUF_LENGTH, sizeof(std::complex<float>));
    this->ft_out = (std::complex<float> *) calloc(BUF_LENGTH, sizeof(std::complex<float>));

    for (auto &buf: this->buffers) buf = (float *) calloc(BUF_LENGTH, sizeof(float));
    for (auto &i: this->found_howls) i = 0;
    // select which algorithm will be run
    setEnableAlgo(settings);
    blackman_win(BUF_LENGTH);
    // fftw3 plan creation
    this->ft_plan = fftwf_plan_dft_1d(BUF_LENGTH, reinterpret_cast<fftwf_complex *>(ft_in),\
    reinterpret_cast<fftwf_complex *>(ft_out), FFTW_FORWARD, FFTW_MEASURE);

}

void Analyzer::blackman_win(int nsamples) {
    int nsamples_periodic = nsamples++;
    for (int i = 0; i < (nsamples_periodic+1)/2; ++i) {
        this->blackman[i]=0.42f -
                           0.5f*cosf(2.0f*PI*(float)i/(nsamples_periodic-1)) +
                           0.08f*cosf(4.0f*PI*(float)i/(nsamples_periodic-1));
        if (i + (nsamples_periodic+1)/2 < nsamples){
            this->blackman[i+(nsamples_periodic+1/2)]= this->blackman[i];
        }
    }
}
/*
 * buf_out = abs(fft(buf_in))
 */
void Analyzer::fftWrapper(const float *buf_in) {
    for (int i = 0; i < BUF_LENGTH; ++i) {
        this->ft_in[i] = buf_in[i]*this->blackman[i];
    }
    fftwf_execute(this->ft_plan);
    for (int i = 0; i < BUF_LENGTH; ++i) {
        this->buffers[0][i] = std::abs(this->ft_out[i]);
    }
}

void Analyzer::updateBuffer(const float *jack_buffer_update) {
    this->jack_buffer = jack_buffer_update;
}
/*
 *  Peak to Harmonics Power Ratio:
 *  Test if the frequency peaks at index[i] has significant harmonics.
 *  Howling and feedbacks usually doesn't generate harmonics waves.
 *  phpr : log10(|Y(wi)|^2/|Y(nwi)|^2)
 *  todo: minimize runtime, even if analysis is not online runtime is critical.
 */
bool Analyzer::phpr(const float *buf_in) {
    bool ret = false;
    for (int i = 0; i < N_PEAKS; ++i) {
        if (this->found_howls[i] != 0){
            ret = true;
            if (i*2 < BUF_LENGTH) {
                if (2 * log10f_fast(buf_in[this->found_howls[i]] / buf_in[this->found_howls[i] * 2])\
                < phpr_threshold) {
                    this->found_howls[i] = 0;
                }
            }
            if (i*3 < BUF_LENGTH){
                if (2 * log10f_fast(buf_in[this->found_howls[i]] / buf_in[this->found_howls[i] * 3])\
                < phpr_threshold) {
                    this->found_howls[i] = 0;
                }
            }
        }
    }
    return ret;
}
/*
 *  Peak to Neighbour Power Ratio
 *  Test if the frequency peaks at index[i] has leakage in neighbours frequencies.
 *  Howling and feedbacks are usually very narrow.
 *  phpr : log10(|Y(wi)|^2/|Y(wi + n(2pi/M)|^2)
 *  todo: minimize runtime, even if analysis is not online runtime is critical.
 */
bool Analyzer::pnpr(const float *buf_in) {
    bool ret = false;
    for (int i = 0; i < N_PEAKS; ++i) {
        if(this->found_howls[i] != 0){
            ret = true;
            if (i+1 < BUF_LENGTH){
                if (2*log10f_fast(buf_in[this->found_howls[i]] / buf_in[this->found_howls[i]+1])\
                < pnpr_threshold) {
                    this->found_howls[i] = 0;
                }
            }
            if (i+2 < BUF_LENGTH) {
                if (2*log10f_fast(buf_in[this->found_howls[i]] / buf_in[this->found_howls[i] + 2])\
                < pnpr_threshold) {
                    this->found_howls[i] = 0;
                }
            }
            if (i-1 > 0){
                if (2*log10f_fast(buf_in[this->found_howls[i]] / buf_in[this->found_howls[i] - 1])\
                < pnpr_threshold) {
                    this->found_howls[i] = 0;
                }
            }
            if (i-2 > 0) {
                if (2*log10f_fast(buf_in[this->found_howls[i]] / buf_in[this->found_howls[i] - 2])\
                < pnpr_threshold) {
                    this->found_howls[i] = 0;
                }
            }
        }
    }
    return ret;
}
/*
 * TODO: Interframe Magnitude Slope Deviation
 */
bool Analyzer::imsd(const float *buf_in) {
    bool ret = true;


    return ret;
}
/*
 * Howls finder:
 * find 10 highest peaks, run phpr, pnpr, imsd if enabled.
 * write howling frequencies indexes in found_howls, if detected.
 */
void Analyzer::run(const float *jackBuffer) {
    updateBuffer(jackBuffer);
    fftWrapper(this->jack_buffer);
    float *buf_in = this->buffers[0];
    for (int & found_howl : this->found_howls) {
        found_howl = 0;
    }
    for (int i = 0; i < BUF_LENGTH; ++i) {
        if (buf_in[i] > buf_in[this->found_howls[0]]){
            this->found_howls[0] = i;
            minHead(buf_in,this->found_howls);
        }
    }
    // recognize larsen effetcs
    if(this->run_pnpr) { if (!pnpr(buf_in)) return; }
    if(this->run_phpr) { if (!phpr(buf_in)) return; }
    if(this->run_imsd) imsd(buf_in);
    // swap buffers order
    float *t = this->buffers[2];
    this->buffers[2]= this->buffers[1];
    this->buffers[1]= this->buffers[0];
    this->buffers[0]=t;
}

void inline Analyzer::minHead(const float *buf_in, int *peaks) {
    int min_peak = 0;
    int tmp = 0;
    for (int i = 0; i < N_PEAKS; ++i) {
        if (buf_in[peaks[i]] < buf_in[peaks[min_peak]]) min_peak = i;
    }
    tmp = peaks[min_peak];
    peaks[min_peak] = peaks[0];
    peaks[0] = tmp;
}

void Analyzer::setEnableAlgo(const bool *settings) {
    this->run_phpr = settings[0];
    this->run_pnpr = settings[1];
    this->run_imsd = settings[2];
}

Analyzer::~Analyzer() {
    fftwf_destroy_plan(this->ft_plan);
    free(ft_in);
    free(ft_out);
    free(found_howls);
    for (auto &buf: this->buffers) {
        free(buf);
    }
    free(buffers);
}

float Analyzer::getPhprThreshold() const {
    return phpr_threshold;
}

float Analyzer::getPnprThreshold() const {
    return pnpr_threshold;
}

float Analyzer::getImsdThreshold() const {
    return imsd_threshold;
}

bool Analyzer::isRunPhpr() const {
    return run_phpr;
}

bool Analyzer::isRunPnpr() const {
    return run_pnpr;
}

bool Analyzer::isRunImsd() const {
    return run_imsd;
}

void Analyzer::setInputBuffer(const float *jackBuffer) {
    jack_buffer = jackBuffer;
}



