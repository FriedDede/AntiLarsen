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

    this->ft_in = (float *) calloc(BUF_LENGTH,sizeof(float));
    this->ft_out = (std::complex<float> *) fftwf_alloc_complex(FFTOUT_BUF_LENGTH);

    for (auto &buf: this->buffers)
        buf = (float *) calloc(FFTOUT_BUF_LENGTH, sizeof(float));

    for (auto &i: this->found_howls) i = 0;

    // select which algorithm will be run
    setEnableAlgo(settings);
    blackman_win(BUF_LENGTH);
    // fftw3 plan creation
    this->ft_plan = fftwf_plan_dft_r2c_1d(BUF_LENGTH, ft_in,\
    reinterpret_cast<fftwf_complex *>(ft_out), FFTW_MEASURE);

}

void Analyzer::blackman_win(int nsamples) {
    int nsamples_periodic = nsamples + 1;
    this->blackman[0]=0.0f;
    for (int i = 1; i < (nsamples_periodic+1)/2; ++i) {
        this->blackman[i]=0.42f -
                           0.5f*cosf(2.0f*PI*(float)i/(nsamples_periodic-1)) +
                           0.08f*cosf(4.0f*PI*(float)i/(nsamples_periodic-1));
        this->blackman[nsamples_periodic-i-1]= this->blackman[i];
    }
}
/*
 * buf_out = abs(fft(jack_buf))
 */
void Analyzer::fftWrapper(const float *jack_buf) {
    for (int i = 0; i < BUF_LENGTH; ++i) {
        this->ft_in[i] = jack_buf[i]* this->blackman[i];
    }
    fftwf_execute(this->ft_plan);
    for (int i = 0; i < (BUF_LENGTH/2)+1; ++i) {
        this->buffers[0][i] = std::abs(this->ft_out[i])/213.0f;
    }
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
            if (i*2 < FFTOUT_BUF_LENGTH) {
                if (20 * log10f_fast(buf_in[this->found_howls[i]] / buf_in[this->found_howls[i] * 2])\
                < phpr_threshold) {
                    this->found_howls[i] = 0;
                }
            }
            if (i*3 < FFTOUT_BUF_LENGTH){
                if (20 * log10f_fast(buf_in[this->found_howls[i]] / buf_in[this->found_howls[i] * 3])\
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
            if (i+1 < FFTOUT_BUF_LENGTH){
                if (20 *log10f_fast(buf_in[this->found_howls[i]] / buf_in[this->found_howls[i]+1])\
                < pnpr_threshold) {
                    this->found_howls[i] = 0;
                }
            }
            if (i-1 > 0){
                if (20 *log10f_fast(buf_in[this->found_howls[i]] / buf_in[this->found_howls[i] - 1])\
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
void Analyzer::analyzeBuffer(const float *jackBuffer) {
    // swap buffers order
    float *t = this->buffers[2];
    this->buffers[2]= this->buffers[1];
    this->buffers[1]= this->buffers[0];
    this->buffers[0]=t;

    setInputBuffer(jackBuffer);
    fftWrapper(this->jack_buffer);
    float *buf_in = this->buffers[0];
    for (int & found_howl : this->found_howls) {
        found_howl = 0;
    }
    for (int i = 0; i < FFTOUT_BUF_LENGTH; ++i) {
        if (buf_in[i] > buf_in[this->found_howls[0]]){
            this->found_howls[0] = i;
            minHead(buf_in,this->found_howls);
        }
    }
    // recognize larsen effetcs
    if(this->run_phpr) { if (!phpr(buf_in)) return; }
    if(this->run_pnpr) { if (!pnpr(buf_in)) return; }
    if(this->run_imsd) imsd(buf_in);
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
    for (auto &buf: this->buffers) {
        free(buf);
    }
}

float Analyzer::getPhprThreshold() const {
    return this->phpr_threshold;
}

float Analyzer::getPnprThreshold() const {
    return this->pnpr_threshold;
}

float Analyzer::getImsdThreshold() const {
    return this->imsd_threshold;
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
    this->jack_buffer = jackBuffer;
}

std::complex<float> *Analyzer::getFtOut() const {
    return ft_out;
}

float *Analyzer::getOutBuffer() const {
    return buffers[0];
}
