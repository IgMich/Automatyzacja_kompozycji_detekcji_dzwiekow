#include "signal_analyzing.h"

float window_hamming(uint16_t idx) {
  float window = 0.54 - 0.46 * cos(TWO_PI * idx / (BUFFER_SIZE - 1));
  return window;
}

Complex *compute_dft(Complex *signal, uint16_t n, float sample_rate, float* frequencies, uint8_t freq_num){
    uint8_t exp = 1;
    float sin[freq_num];
    float cos[freq_num];
    for(uint8_t i = 0;i<freq_num;i++){
        sin[i]=cos[i]=0.0;
    }
    for(uint16_t i=0;i<n;i++){
        for(int h=0;h<freq_num;h++){
            uint8_t exp = 1 << (h/12);
            uint8_t idx = h%12;
            uint16_t tuning_word = (uint16_t)(i*frequencies[idx]*exp*LUT_SIZE/sample_rate)%LUT_SIZE;
            sin[h] += signal[i].real()*2.0f*(float)lut[tuning_word]/(SIN_MAX*1.0f);
            cos[h] += signal[i].real()*2.0f*(float)lut[(tuning_word+COS_SHIFT)%LUT_SIZE]/(SIN_MAX*1.0f);
        }
    }
    Complex* result = allocate_complex_array(freq_num);
    for(int h=0;h<freq_num;h++){
        sin[h]/=n*1.0f;
        cos[h]/=n*1.0f;
        result[h] = sqrtf((sin[h]*sin[h]+cos[h]*cos[h]))/sqrtf(2.0);
    }
    return result;
}

void display_dft(float* magnitude,  float* frequencies,  uint8_t freq_num){
    float max_mag = 0;
    for (int i = 0; i < freq_num; i++) {
        if (magnitude[i] > max_mag) max_mag = magnitude[i];
    }
    Serial.printf("\nFrequency Spectrum:\n");
    Serial.printf("==================\n");
    
    // Display each frequency bin
    for (int i = 0; i < freq_num; i++) {
        int idx = i%12;
        int exp = 1<<(i/12);
        float freq = frequencies[idx]*exp*(1.0f);
        int bar_length = (int)(50 * magnitude[i] / max_mag);
        
        Serial.printf("%5.0f Hz |", freq);
        for (int j = 0; j < bar_length; j++) {
            Serial.printf("[]");
        }
        Serial.printf(" %.3f\n", magnitude[i]);
    }   
}

float bin_to_frequency(uint16_t bin) {
    return bin * SAMPLE_RATE / BUFFER_SIZE;
}

void display_spectrum_ascii(float* magnitude) {
    int display_bins = 64;  // Number of bins to display
    if (display_bins > BUFFER_SIZE/2) display_bins = BUFFER_SIZE/2;
    
    // Find maximum magnitude for scaling
    float max_mag = 0;
    for (int i = 0; i < display_bins; i++) {
        if (magnitude[i] > max_mag) max_mag = magnitude[i];
    }
    
    printf("\nFrequency Spectrum:\n");
    printf("==================\n");
    
    // Display each frequency bin
    for (int i = 0; i < display_bins; i++) {
        float freq = bin_to_frequency(i);
        int bar_length = (int)(50 * magnitude[i] / max_mag);
        
        printf("%5.0f Hz |", freq);
        for (int j = 0; j < bar_length; j++) {
            printf("█");
        }
        printf(" %.3f\n", magnitude[i]);
    }
}