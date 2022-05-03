//
// Created by andre on 4/26/22.
//

#include "../include/peaksFinder.h"
#include "../include/fastmath.h"
#include <cmath>
#include <complex>
#include <fftw3.h>

peaksFinder::peaksFinder(const float *jack_buffer, const bool settings[3]){
    this->jack_buffer = jack_buffer;
    found_howls = (int *) calloc(N_PEAKS,sizeof(int));
    ft_in = (std::complex<float> *) calloc(BUF_LENGTH, sizeof(std::complex<float>));
    ft_in = (std::complex<float> *) calloc(BUF_LENGTH, sizeof(std::complex<float>));
    for (auto &buf: this->buffers) {
        buf = (float *) calloc(BUF_LENGTH, sizeof(float));
    }
    algoSelect(settings);
    this->ft_plan = fftw_plan_dft_1d(BUF_LENGTH, reinterpret_cast<fftw_complex *>(ft_in),\
    reinterpret_cast<fftw_complex *>(ft_out), FFTW_FORWARD, FFTW_MEASURE);
    fftWrapper(jack_buffer, this->buffers[0]);
}

void peaksFinder::updateBuffer() {
    fftWrapper(jack_buffer, this->buffers[current_buffer]);
    this->current_buffer = (this->current_buffer == 2) ? 0 : this->current_buffer++;
}
/*
 *  Peak to Harmonics Power Ratio:
 *  Test if the frequency peaks at index[i] has significant harmonics.
 *  Howling and feedbacks usually doesn't generate harmonics waves.
 *  phpr : log10(|Y(wi)|^2/|Y(nwi)|^2)
 */
void peaksFinder::phpr(const float *buf_in, int peaks[N_PEAKS]) {
    for (int i = 0; i < N_PEAKS; ++i) {
        if (peaks[i] != 0 and i*2 < BUF_LENGTH){
            if (log10f_fast((pow(buf_in[peaks[i]],2))/(pow(buf_in[peaks[i]*2],2)))\
            < phpr_threshold) {
                peaks[i] = 0;
            }
            else if (i*3 < BUF_LENGTH){
                if (log10f_fast((pow(buf_in[peaks[i]],2))/(pow(buf_in[peaks[i]*3],2)))\
                < phpr_threshold) {
                    peaks[i] = 0;
                }
            }
        }
    }
}
/*
 * Peak to Neighbour Power Ratio
 *  Test if the frequency peaks at index[i] has leakage in neighbours frequencies.
 *  Howling and feedbacks are usually very narrow.
 *  phpr : log10(|Y(wi)|^2/|Y(wi + n(2pi/M)|^2)
 */
void peaksFinder::pnpr(const float *buf_in, int peaks[N_PEAKS]) {
    for (int i = 0; i < N_PEAKS; ++i) {
        if(peaks[i] != 0){
            if (i+1 < BUF_LENGTH){
                if (2*log10f_fast(buf_in[peaks[i]])-2*log10f_fast((buf_in[peaks[i]+1]))\
                < pnpr_threshold) {
                    peaks[i] = 0;
                }
            }
            if (i+2 < BUF_LENGTH) {
                if (log10f_fast((pow(buf_in[peaks[i]], 2)) / (pow(buf_in[peaks[i] + 2], 2)))\
                < pnpr_threshold) {
                    peaks[i] = 0;
                }
            }
            if (i-1 > 0){
                if (log10f_fast((pow(buf_in[peaks[i]],2))/(pow(buf_in[peaks[i]-1],2)))\
                < pnpr_threshold) {
                    peaks[i] = 0;
                }
            }
            if (i-2 > 0) {
                if (log10f_fast((pow(buf_in[peaks[i]], 2)) / (pow(buf_in[peaks[i] - 2], 2)))\
                < pnpr_threshold) {
                    peaks[i] = 0;
                }
            }
        }
    }
}
/*
 * TODO: Interframe Magnitude Slope Deviation
 */
void peaksFinder::imsd(const float *buf_in, int *peaks) {

}
/*
 * TODO: Implement some kind of fast ft
 */
void peaksFinder::fftWrapper(const float *buf_in, float *buf_out) {
    for (int i = 0; i < BUF_LENGTH; ++i) {
        this->ft_in[i] = buf_in[i];
    }
    fftw_execute(this->ft_plan);
    for (int i = 0; i < BUF_LENGTH; ++i) {
        buffers[this->current_buffer][i] = std::abs(this->ft_out[i]);
    }
}
/*
 * TODO: Howls finder, find 10 highest peaks, run phpr, pnpr, imsd on them
 * TODO: write howling indexes in found_howls if found
 */
void peaksFinder::run() {
    int * peaks = this->found_howls;
    float *buf_in = this->buffers[this->current_buffer];

    for (int i = 0; i < N_PEAKS; ++i) {
        peaks[i] = 0;
    }
    for (int i = 0; i < BUF_LENGTH; ++i) {
        if (buf_in[i] > buf_in[peaks[0]]){
            peaks[0] = i;
            minHead(buf_in,peaks);
        }
    }
    if(this->run_phpr) phpr(buf_in,peaks);
    if(this->run_pnpr) pnpr(buf_in,peaks);
    if(this->run_imsd) imsd(buf_in,peaks);
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

void peaksFinder::algoSelect(const bool settings[3]) {
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
}

