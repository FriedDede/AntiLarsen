//
// Created by andre on 5/3/22.
//

#ifndef JACK_ANTILARSEN_FASTMATH_H
#define JACK_ANTILARSEN_FASTMATH_H


float log2f_approx(float);
//log10f is exactly log2(x)/log2(10.0f)
#define log10f_fast(x)  (log2f_approx(x)*0.3010299956639812f)
#endif
