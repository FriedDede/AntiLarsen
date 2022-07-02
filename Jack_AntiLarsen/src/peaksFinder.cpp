//
// Created by andre on 4/26/22.
//

#include "../include/peaksFinder.h"
#include "../include/fastmath.h"
#include <cmath>
#include <complex>
#include <fftw3.h>

peaksFinder::peaksFinder(const bool settings[3]){
    // locate jack audio buffer
    this->jack_buffer = nullptr;
    /*
     * todo: test fftw_complex *fftw_malloc() for SSE/AVX speedup
     */
    ft_in = (std::complex<float> *) calloc(BUF_LENGTH, sizeof(std::complex<float>));
    ft_out = (std::complex<float> *) calloc(BUF_LENGTH, sizeof(std::complex<float>));
    for (auto &buf: this->buffers) buf = (float *) calloc(BUF_LENGTH, sizeof(float));
    for (auto &i: this->found_howls) i = 0;
    // select which algorithm will be run
    setEnableAlgo(settings);
    // fftw3 plan creation
    this->ft_plan = fftw_plan_dft_1d(BUF_LENGTH, reinterpret_cast<fftw_complex *>(ft_in),\
    reinterpret_cast<fftw_complex *>(ft_out), FFTW_FORWARD, FFTW_MEASURE);
}

void peaksFinder::updateBuffer(const float *jack_buffer_update) {
    this->jack_buffer = jack_buffer_update;
    fftWrapper(this->jack_buffer, this->buffers[current_buffer_idx]);
    this->current_buffer_idx = (this->current_buffer_idx == 2) ? 0 : this->current_buffer_idx++;
}
/*
 *  Peak to Harmonics Power Ratio:
 *  Test if the frequency peaks at index[i] has significant harmonics.
 *  Howling and feedbacks usually doesn't generate harmonics waves.
 *  phpr : log10(|Y(wi)|^2/|Y(nwi)|^2)
 *  todo: minimize runtime, even if analysis is not online runtime is critical.
 */
bool peaksFinder::phpr(const float *buf_in) {
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
bool peaksFinder::pnpr(const float *buf_in) {
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
bool peaksFinder::imsd(const float *buf_in) {
    bool ret = true;


    return ret;
}
/*
 * buf_out = abs(fft(buf_in))
 */
void peaksFinder::fftWrapper(const float *buf_in, float *buf_out) {
    for (int i = 0; i < BUF_LENGTH; ++i) {
        this->ft_in[i] = buf_in[i];
    }
    fftw_execute(this->ft_plan);
    for (int i = 0; i < BUF_LENGTH; ++i) {
        buffers[this->current_buffer_idx][i] = std::abs(this->ft_out[i]);
    }
}
/*
 * Howls finder:
 * find 10 highest peaks, run phpr, pnpr, imsd if enabled.
 * write howling frequencies indexes in found_howls, if detected.
 */
void peaksFinder::run(const float *jackBuffer) {
    updateBuffer(jackBuffer);
    int * peaks = this->found_howls;
    float *buf_in = this->buffers[this->current_buffer_idx];

    for (int i = 0; i < N_PEAKS; ++i) {
        peaks[i] = 0;
    }
    for (int i = 0; i < BUF_LENGTH; ++i) {
        if (buf_in[i] > buf_in[peaks[0]]){
            peaks[0] = i;
            minHead(buf_in,peaks);
        }
    }
    if(this->run_phpr) { if (!phpr(buf_in)) return; }
    if(this->run_pnpr) { if (!pnpr(buf_in)) return; }
    if(this->run_imsd) imsd(buf_in);
}

void inline peaksFinder::minHead(const float *buf_in, int *peaks) {
    int min_peak = 0;
    int tmp = 0;
    for (int i = 0; i < N_PEAKS; ++i) {
        if (buf_in[peaks[i]] < buf_in[peaks[min_peak]]) min_peak = i;
    }
    tmp = peaks[min_peak];
    peaks[min_peak] = peaks[0];
    peaks[0] = tmp;
}

void peaksFinder::setEnableAlgo(const bool *settings) {
    this->run_phpr = settings[0];
    this->run_pnpr = settings[1];
    this->run_imsd = settings[2];
}

peaksFinder::~peaksFinder() {
    fftw_destroy_plan(this->ft_plan);
    free(ft_in);
    free(ft_out);
    free(found_howls);
    for (auto &buf: this->buffers) {
        free(buf);
    }
    free(buffers);
}

void peaksFinder::setPhprThreshold(float phprThreshold) {
    this->phpr_threshold = phprThreshold;
}

void peaksFinder::setPnprThreshold(float pnprThreshold) {
    this->pnpr_threshold = pnprThreshold;
}

void peaksFinder::setImsdThreshold(float imsdThreshold) {
    this->imsd_threshold = imsdThreshold;
}

float peaksFinder::getPhprThreshold() const {
    return phpr_threshold;
}

float peaksFinder::getPnprThreshold() const {
    return pnpr_threshold;
}

float peaksFinder::getImsdThreshold() const {
    return imsd_threshold;
}

bool peaksFinder::isRunPhpr() const {
    return run_phpr;
}

bool peaksFinder::isRunPnpr() const {
    return run_pnpr;
}

bool peaksFinder::isRunImsd() const {
    return run_imsd;
}

void peaksFinder::setInputBuffer(const float *jackBuffer) {
    jack_buffer = jackBuffer;
}

