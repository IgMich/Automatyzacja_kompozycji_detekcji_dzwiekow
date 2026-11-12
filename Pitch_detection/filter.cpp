#include "filter.h"

static float xv[NZEROS+1], yv[NPOLES+1];

float Butterworth_filter(float in)
  {
    float out;
    xv[0] = xv[1]; xv[1] = xv[2]; xv[2] = xv[3]; xv[3] = xv[4]; 
    xv[4] = in/ GAIN;
    yv[0] = yv[1]; yv[1] = yv[2]; yv[2] = yv[3]; yv[3] = yv[4]; 
    yv[4] =   (xv[0] + xv[4]) - 2 * xv[2]
                 + ( -0.7603180775 * yv[0]) + (  3.2461595636 * yv[1])
                 + ( -5.2110927366 * yv[2]) + (  3.7252501169 * yv[3]);
    out=yv[4];
    return out;  
  }

void reset_filter()
{
    for (uint8_t i = 0; i <= NZEROS; i++) {
        xv[i] =0.0f;
    }
    for (uint8_t i = 0; i <= NPOLES; i++) {
        yv[i] = 0.0f;
    }
}