#ifndef FILTER_H
#define FILTER_H
#include "common.h"

#define NZEROS 4
#define NPOLES 4
#define GAIN   1.185418179e+02

float Butterworth_filter(float in);
void reset_filter();
#endif