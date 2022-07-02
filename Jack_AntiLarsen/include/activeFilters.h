//
// Created by posta on 7/1/2022.
//
#include "iir.h"
#ifndef JACK_ANTILARSEN_ACTIVEFILTERS_H
#define JACK_ANTILARSEN_ACTIVEFILTERS_H
/*
 * active filters bank
 */
typedef struct bank{
    t_filter *p_filter[MAX_ACTIVE_FILTERS];
    int active_filters = 0;
    int next_insert = 0;
}t_bank;
/*
 * Active filters class
 * Stores pointers to active p_filter
 * Apply p_filter to audio frame
 */
class activeFilters{
public:
    activeFilters();
    bool applyFilters();
    void reset_bank();
    void add_filter_to_bank(int index,t_filter*);
    t_bank bank;
    void setInOutBuffers(float *in, float* out, int nframes);
    float *getBufOut() const;
private:
    float *buf_in;
    float *buf_out;
    int nframes = 0;
};

#endif
