#ifndef SIGNAL_ANALYZING_H
#define SIGNAL_ANALYZING_H

#include "common.h"
#include "lookuptable.h"
#include "fft_algorithms.h"
#define BUFFER_SIZE 4096
#define SAMPLE_RATE 44100

static inline float* compute_magnitude(Complex* result, uint16_t n) {
    float* mag = (float*)malloc(n*sizeof(float));
    
    for (int i = 0; i < n; i++) {
        mag[i] = result[i].modulus();
    }
    return mag;
}

float window_hamming(uint16_t idx);
Complex *compute_dft(Complex *signal, uint16_t n, float sample_rate,  float* frequencies,  uint8_t freq_num);
void display_dft(float* magnitude,  float* frequencies,  uint8_t freq_num);
float bin_to_frequency(uint16_t bin);
void display_spectrum_ascii(float* magnitude);
#endif
