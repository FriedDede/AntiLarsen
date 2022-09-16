//
// Created by andre on 9/16/22.
//

#ifndef JACK_ANTILARSEN_RTA_H
#define JACK_ANTILARSEN_RTA_H

#include "client.h"
#include "vector"
#include "complex"
#include "fftw3.h"
#include "const.h"


class rta:public client {
public:
    explicit rta(const char *name);
    int process(jack_nframes_t, void *);
    void setFftLength(int fftLength);
    int getFftLength() const;
    std::complex<float> *getFtOut() const;

protected:
    int head_buffer = 0;
    int fft_length = 5;
    float *buffers[5];
    float ft_in[BUF_LENGTH*5];
    std::complex<float> *ft_out;
    fftwf_plan ft_plan;
};


#endif //JACK_ANTILARSEN_RTA_H
