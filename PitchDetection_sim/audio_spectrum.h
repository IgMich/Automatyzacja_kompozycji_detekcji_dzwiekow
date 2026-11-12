#ifndef AUDIO_SPECTRUM_H
#define AUDIO_SPECTRUM_H
#include "fft_common.h"
#include "fft_algorithms.h"
#include <stddef.h>


#define NZEROS 4
#define NPOLES 4
#define GAIN   1.185418179e+02


// # MIT License

// Copyright (c) 2024 FFT Study Repository Contributors

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

// Find peak frequencies in spectrum
typedef struct {
    float frequency;
    float magnitude;
    int bin;
} peak_t;

void filterloop(complex_t* in,complex_t *out, int n);
void reset_filter();
void apply_window_hann(complex_t* signal, int n);
void apply_window_hamming(complex_t* signal, int n);
void apply_window_blackman(complex_t* signal, int n);
complex_t *compute_dft(complex_t* signal, int n, float sample_rate, float* frequencies, int freq_num);
void generate_test_audio(complex_t* signal, int n, float sample_rate);
float bin_to_frequency(int bin, int fft_size, float sample_rate);
void generate_sin_dds(complex_t *signal, int n, float sample_freq, float *freq, int freq_num);
void find_peaks(float* magnitude, int n, float sample_rate, 
                peak_t* peaks, int* num_peaks, int max_peaks);
void display_spectrum_ascii(float* magnitude, int n, float sample_rate);
void display_dft(float* magnitude, int n, float sample_rate,float* frequencies, int freq_num);
void analyze_audio_spectrum(complex_t* signal, int n, float sample_rate, 
                          const char* window_type);


#endif