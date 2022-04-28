//
// Created by andre on 4/26/22.
//

#include "../include/peaksFinder.h"

peaksFinder::peaksFinder(float *buffer){
    this->jack_buffer = buffer;
    fftWrapper(buffer, this->buffer[0]);
    for (auto &p: found_howls) {
        p = 0;
    }
}

void peaksFinder::updateBuffer() {
    fftWrapper(jack_buffer, this->buffer[current_buffer]);
    this->current_buffer = (this->current_buffer == 2) ? 0 : this->current_buffer++;
}
/*
 *  TODO: Peak to Harmonics Power Ratio
 */
void peaksFinder::phpr(const float *buf_in, int *peaks) {

}
/*
 * TODO: Peak to Neighbour Power Ratio
 */
void peaksFinder::pnpr(const float *buf_in, int *peaks) {

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

}
/*
 * TODO: Howls finder, find 10 highest peaks, run phpr, pnpr, imsd on them
 * TODO: write howling indexes in found_howls if found
 */
void peaksFinder::findHowls(const float *buf_in, int *peaks) {
    for (int i = 0; i < N_PEAKS; ++i) {
        peaks[i] = 0;
    }
    for (int i = 0; i < BUF_LENGTH; ++i) {
        if (buf_in[i] > buf_in[peaks[0]]) peaks[0] = i;
        minHead(buf_in,peaks);
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

