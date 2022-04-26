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
void peaksFinder::phpr() {

}
/*
 * TODO: Peak to Neighbour Power Ratio
 */
void peaksFinder::pnpr() {

}
/*
 * TODO: Interframe Magnitude Slope Deviation
 */
void peaksFinder::imsd() {

}
/*
 * TODO: Implement some kind of fast ft
 */
void peaksFinder::fftWrapper(const float *src, float *dest) {

}
/*
 * TODO: Howls finder, find 10 highest peaks, run phpr, pnpr, imsd on them
 * TODO: write howling indexes in found_howls if found
 */
void peaksFinder::findHowls() {

}

