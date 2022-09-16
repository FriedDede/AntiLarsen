//
// Created by andre on 9/16/22.
//

#include "../include/rta.h"

rta::rta(const char *name) : client(name) {
    this->type = mag_rta;
    //this->ft_in = (float *) calloc(BUF_LENGTH*fft_length,sizeof(float));
    this->ft_out = (std::complex<float> *) fftwf_alloc_complex((FFTOUT_BUF_LENGTH)*fft_length);

    //buffers = (float**) malloc(fft_length * sizeof(float*));
    for (int i = 0; i < fft_length; ++i)
        buffers[i] = (float *) calloc(BUF_LENGTH, sizeof(float));

    // fftw3 plan creation
    this->ft_plan = fftwf_plan_dft_r2c_1d(BUF_LENGTH*fft_length, ft_in,\
    reinterpret_cast<fftwf_complex *>(ft_out), FFTW_MEASURE);

                                          this->unregisterPort(jack_port_name(output_port));
}

int rta::process(jack_nframes_t frames, void*arg) {
    float* jack_buf = (float *)jack_port_get_buffer(input_port,frames);
    for (int i = 0; i < BUF_LENGTH; ++i) {
        this->buffers[0][i] = jack_buf[i];
    }
    for (int i = 0; i < fft_length; ++i) {
        for (int j = 0; j < BUF_LENGTH; ++j) {
            ft_in[i*j] = buffers[j][i];
        }
    }
    fftwf_execute(ft_plan);
    return 0;
}

std::complex<float> *rta::getFtOut() const {
    return ft_out;
}

void rta::setFftLength(int fftLength) {
    fft_length = fftLength;
}
int rta::getFftLength() const {
    return fft_length;
}
