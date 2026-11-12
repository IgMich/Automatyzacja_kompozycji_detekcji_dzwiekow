#ifndef FFT_COMMON_H
#define FFT_COMMON_H

#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <assert.h>

// Compiler optimization hints
#ifdef __GNUC__
    #define LIKELY(x)   __builtin_expect(!!(x), 1)
    #define UNLIKELY(x) __builtin_expect(!!(x), 0)
    #define FORCE_INLINE __attribute__((always_inline)) inline
#else
    #define LIKELY(x)   (x)
    #define UNLIKELY(x) (x)
    #define FORCE_INLINE inline
#endif



// FFT direction
typedef enum {
    FFT_FORWARD = -1,
    FFT_INVERSE = 1
} fft_direction;

// Common utility functions
static FORCE_INLINE int is_power_of_two(int n) {
    return LIKELY(n > 0) && ((n & (n - 1)) == 0);
}

static inline int next_power_of_two(int n) {
    n--;
    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;
    n++;
    return n;
}

static inline int log2_int(int n) {
    int log = 0;
    while (n >>= 1) log++;
    return log;
}

// Bit reversal function with optimization for small sizes
static inline unsigned int bit_reverse(unsigned int x, int log2n) {
    // Optimized bit reversal for common small sizes
    if (log2n <= 8) {
        // Use bit manipulation tricks for small sizes
        x = ((x & 0xAAAA) >> 1) | ((x & 0x5555) << 1);
        x = ((x & 0xCCCC) >> 2) | ((x & 0x3333) << 2);
        x = ((x & 0xF0F0) >> 4) | ((x & 0x0F0F) << 4);
        if (log2n > 4) x = ((x & 0xFF00) >> 8) | ((x & 0x00FF) << 8);
        return x >> (16 - log2n);
    }
    
    // General case for larger sizes
    unsigned int r = 0;
    for (int i = 0; i < log2n; i++) {
        r = (r << 1) | (x & 1);
        x >>= 1;
    }
    return r;
}

static inline void free_complex_array(Complex* arr) {
    if (arr) free(arr);
}

// Twiddle factor computation with optimization for common cases
static inline Complex twiddle_factor(int k, int n, fft_direction dir) {
    // Optimize for common cases
    if (k == 0)  return Complex(1,0);
    if (k * 4 == n) return (dir == FFT_FORWARD) ? Complex(0,-1) : Complex(0,1);  // ±j
    if (k * 2 == n) return Complex(-1,0);  // -1
    if (k * 4 == 3 * n) return (dir == FFT_FORWARD) ? Complex(0,1) : Complex(0,-1);  // ∓j
    
    float angle = static_cast<int>(dir) * TWO_PI * k / n;
    Complex tmp = Complex(0,angle);
    return tmp.c_exp();
}

// Performance timing (renamed to avoid conflict with system timer_t)
typedef struct {
    clock_t start;
    clock_t end;
    float elapsed_ms;
} fft_timer_t;

static inline void timer_start(fft_timer_t* timer) {
    timer->start = clock();
}

static inline void timer_stop(fft_timer_t* timer) {
    timer->end = clock();
    timer->elapsed_ms = ((float)(timer->end - timer->start) / CLOCKS_PER_SEC) * 1000.0;
}

// Error checking
#define CHECK_NULL(ptr, msg) \
    if (!(ptr)) { \
        fprintf(stderr, "Error: %s\n", msg); \
        exit(EXIT_FAILURE); \
    }

#define CHECK_POWER_OF_TWO(n) \
    if (!is_power_of_two(n)) { \
        fprintf(stderr, "Error: Size %d is not a power of two\n", n); \
        exit(EXIT_FAILURE); \
    }


// Signal generation utilities
static inline void generate_sine_wave(Complex* signal, int n, float freq, float fs) {
    for (int i = 0; i < n; i++) {
        signal[i].set(sin(TWO_PI * freq * i / fs),0);
    }
}

static inline void generate_square_wave(Complex* signal, int n, float freq, float fs) {
    int period = (int)(fs / freq);
    for (int i = 0; i < n; i++) {
        signal[i] = (i % period < period / 2) ? 1.0 : -1.0;
    }
}

static inline void generate_impulse(Complex* signal, int n) {
    memset(signal, 0, n * sizeof(Complex));
    signal[0] = 1.0;
}




#endif // FFT_COMMON_H
