//
// Created by andre on 5/7/22.
//

#ifndef JACK_ANTILARSEN_LOCALTOOLS_H
#define JACK_ANTILARSEN_LOCALTOOLS_H
#include <cstring>
/*
 * Local string copy
 */
static inline char * localStrdup(const char * input) {
    char * output = new char[strlen(input) + 1];
    strcpy(output, input);
    return output;
}

#endif
