//
// Created by andre on 9/16/22.
//

#include "../include/rta.h"

rta::rta(const char *name) : client(name) {
    this->type = mag_rta;
    //this->ft_in = (float *) calloc(BUF_LENGTH*fft_length,sizeof(float));
    this->ft_out = (std::complex<float> *) fftwf_alloc_complex((512*fft_length)+1);

    //buffers = (float**) malloc(fft_length * sizeof(float*));
    for (int i = 0; i < fft_length; ++i)
        buffers[i] = (float *) calloc(BUF_LENGTH, sizeof(float));

    // fftw3 plan creation
    this->ft_plan = fftwf_plan_dft_r2c_1d(\
            BUF_LENGTH*fft_length,ft_in,\
            reinterpret_cast<fftwf_complex *>(ft_out), FFTW_MEASURE);

    this->unregisterPort(jack_port_name(output_port));
    blackman_win(BUF_LENGTH);
}

int rta::process(jack_nframes_t frames, void*arg) {
    //get jack buffer pointer
    float* jack_buf = (float *)jack_port_get_buffer(input_port,frames);
    //place new jack buffer in ring buffer head
    for (int i = 0; i < BUF_LENGTH; ++i) {
        this->buffers[0][i] = jack_buf[i];
    }
    //copy ring buffer content in contiguous reordered buffer for fft
    int c = fft_length-1;
    for (int i = 0; i < fft_length ; ++i) {
        for (int j = 0; j < BUF_LENGTH; ++j) {
            this->ft_in[(BUF_LENGTH*i)+j] = this->buffers[c][j] * this->blackman[j];
        }
        --c;
    }
    fftwf_execute(ft_plan);
    //swap ring buffer pointers
    float *tmp = this->buffers[fft_length-1];
    for (int i = fft_length-1; i > 0 ; --i) {
        this->buffers[i] = this->buffers[i-1];
    }
    this->buffers[0] = tmp;
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
void rta::blackman_win(int nsamples) {
    int nsamples_periodic = nsamples + 1;
    this->blackman[0]=0.0f;
    for (int i = 1; i < (nsamples_periodic+1)/2; ++i) {
        this->blackman[i]=0.42f -
                            0.5f*cosf(2.0f*PI*(float)i/(float)(nsamples_periodic-1)) +
                            0.08f*cosf(4.0f*PI*(float)i/(float)(nsamples_periodic-1));
        this->blackman[nsamples_periodic-i-1]= this->blackman[i];
    }
}