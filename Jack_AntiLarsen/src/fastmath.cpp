//
// Created by andre on 5/3/22.
//

#include "../include/fastmath.h"
#include "cmath"
// This is a fast approximation to log2()
// Y = C[0]*F*F*F + C[1]*F*F + C[2]*F + C[3] + E;
float log2f_approx(const float X) {
    float Y, F;
    int E;
    F = frexpf(fabsf(X), &E);
    Y = 1.23149591368684f;
    Y *= F;
    Y += -4.11852516267426f;
    Y *= F;
    Y += 6.02197014179219f;
    Y *= F;
    Y += -3.13396450166353f;
    Y += (float)E;
    return(Y);
}
//log10f as log2(x)/log2(10.0f)
float log10f_fast(const float X){
    return log2f_approx(X)*0.3010299956639812f;
};