//
// Created by andre on 7/7/22.
//
#include <iostream>
#include <jack/jack.h>
#include <jack/types.h>
#include "include/Analyzer.h"
#include "include/DSP.h"
#include "include/const.h"
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <chrono>

DSP *dsp;
Analyzer *analyzer;

int main(){
    float out_buf_test[BUF_LENGTH];
    int ntests = 3;
    bool settings[3];
    int test_peaks[N_PEAKS];
    settings[0] = true;
    settings[1] = false;
    settings[2] = false;
    bool larsen;
    const static float fsampling = 48000.0f;
    dsp = new DSP;
    analyzer = new Analyzer(settings);

    float test[ntests][BUF_LENGTH] ;
    for (int i = 0; i < BUF_LENGTH; ++i) {
        test[0][i] = 0.1f * cosf((float)i*2.0f*PI*42.0f*FSTEP/fsampling);
    }
    for (int i = 0; i < BUF_LENGTH; ++i) {
        test[1][i] = 0.2f * cosf((float)i*2.0f*PI*42.0f*FSTEP/fsampling);
    }
    for (int i = 0; i < BUF_LENGTH; ++i) {
        test[2][i] = 0.9f * cosf((float)i*2.0f*PI*42.f*FSTEP/fsampling);
    }
    if (analyzer->isRunPhpr()) std::cout << "phpr ";
    if (analyzer->isRunPnpr()) std::cout << "pnpr ";
    if (analyzer->isRunImsd()) std::cout << "imsd ";
    std::cout << std::endl;
    for (int i = 0; i < ntests; ++i) {
        analyzer->analyzeBuffer(test[i]);
        std::cout << "test " << i << ":     ";
        for (auto &n: analyzer->found_howls) {
            std::cout << n << " ";
        }
        memcpy(test_peaks,analyzer->found_howls, 10*sizeof(int));
        std::cout << std::endl;
        std::cout << "peaks:    ";
        for (auto &n: analyzer->found_howls) {
            if (n != 0)std::cout << (analyzer->getOutBuffer()[n]) << " ";
        }
        std::cout << std::endl;
        std::cout << "filtered:  ";
        analyzer->analyzeBuffer(test[i]);
        dsp->setInOutBuffers(test[i],out_buf_test,BUF_LENGTH);
        for (auto f_idx: analyzer->found_howls) {
            if (f_idx != 0){
                dsp->add_filter_to_bank(f_idx);
                larsen = true;
            }
        }
        if (larsen) dsp->applyFilters();
        dsp->reset_bank();
        analyzer->analyzeBuffer(out_buf_test);
        larsen = false;
        for (auto &n: test_peaks) {
            if (n != 0)std::cout << analyzer->getOutBuffer()[n] << " ";
        }
        std::cout << std::endl;
    }

    // Timing test
    for (int i = 0; i < ntests; ++i) {
        auto begin = std::chrono::high_resolution_clock::now();
        analyzer->analyzeBuffer(test[i]);
        dsp->setInOutBuffers(test[i],out_buf_test,BUF_LENGTH);
        for (auto f_idx: analyzer->found_howls) {
            if (f_idx != 0){
                dsp->add_filter_to_bank(f_idx*2);
                larsen = true;
            }
        }
        if (larsen) dsp->applyFilters();
        larsen = false;
        dsp->reset_bank();
        auto end = std::chrono::high_resolution_clock::now();
        std::cout << "test " << i << " time:    "<< \
    std::chrono::duration_cast<std::chrono::nanoseconds>(end-begin).count() \
    << "ns" << std::endl;
    }

    delete analyzer;
    delete dsp;
    return 0;
}