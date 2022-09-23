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
    int process(jack_nframes_t, void *) override;
    void setFftLength(int fftLength);
    int getFftLength() const;
    std::complex<float> *getFtOut() const;
    void blackman_win(int);

protected:
    int head_buffer = 0;
    int fft_length = 4;
    float *buffers[4];
    float ft_in[BUF_LENGTH*4];
    std::complex<float> *ft_out;
    fftwf_plan ft_plan;
    float blackman[BUF_LENGTH];
};


#endif //JACK_ANTILARSEN_RTA_H
