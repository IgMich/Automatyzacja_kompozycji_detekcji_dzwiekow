#include "audio_spectrum.h"
#include "lookuptable.h"


/* Digital filter designed by mkfilter/mkshape/gencode   A.J. Fisher
   Command line: /var/www/html/anteeo.se/public_html/fisher/helpers/mkfilter -Bu -Bp -o 5 -a 9.0702947846e-04 3.6281179138e-02 -l */

static double xv[NZEROS+1], yv[NPOLES+1];

void filterloop(complex_t *in, complex_t *out, int n)
  { for (int i=0;i<n;i++)
      { xv[0] = xv[1]; xv[1] = xv[2]; xv[2] = xv[3]; xv[3] = xv[4]; 
        xv[4] =  (double)in[i]/ GAIN;
        yv[0] = yv[1]; yv[1] = yv[2]; yv[2] = yv[3]; yv[3] = yv[4]; 
        yv[4] =   (xv[0] + xv[4]) - 2 * xv[2]
                     + ( -0.7603180775 * yv[0]) + (  3.2461595636 * yv[1])
                     + ( -5.2110927366 * yv[2]) + (  3.7252501169 * yv[3]);
        out[i] = (complex_t)yv[4];
      }
  }

// =============================================================
//          ZEROWANIE BUFORÓW STANU
// =============================================================
void reset_filter()
{
    for (int i = 0; i <= NZEROS; i++) {
        xv[i] =0.0f;
    }
    for (int i = 0; i <= NPOLES; i++) {
        yv[i] = 0.0f;
    }
}

// Window functions for spectral analysis
void apply_window_hann(complex_t* signal, int n) {
    for (int i = 0; i < n; i++) {
        float window = 0.5 * (1.0 - cos(TWO_PI * i / (n - 1)));
        signal[i] *= window;
    }
}

void apply_window_hamming(complex_t* signal, int n) {
    for (int i = 0; i < n; i++) {
        float window = 0.54 - 0.46 * cos(TWO_PI * i / (n - 1));
        signal[i] *= window;
    }
}

void apply_window_blackman(complex_t* signal, int n) {
    for (int i = 0; i < n; i++) {
        float window = 0.42 - 0.5 * cos(TWO_PI * i / (n - 1)) 
                      + 0.08 * cos(4 * PI * i / (n - 1));
        signal[i] *= window;
    }
}

complex_t *compute_dft(complex_t *signal, int n, float sample_rate, float* frequencies, int freq_num){
    int exp = 1;
    float *sin = calloc(freq_num,sizeof(float));
    float *cos = calloc(freq_num,sizeof(float));
    for(int i=0;i<n;i++){
        for(int h=0;h<freq_num;h++){
            int exp = 1 << (h/12);
            int idx = h%12;
            uint16_t tuning_word = (uint16_t)(i*frequencies[idx]*exp*LUT_SIZE/sample_rate)%LUT_SIZE;
            sin[h] += signal[i]*2.0f*(float)lut[tuning_word]/(SIN_MAX*1.0f);
            cos[h] += signal[i]*2.0f*(float)lut[(tuning_word+COS_SHIFT)%LUT_SIZE]/(SIN_MAX*1.0f);
        }
    }
    complex_t *result = allocate_complex_array(freq_num);
    for(int h=0;h<freq_num;h++){
        sin[h]/=n*1.0f;
        cos[h]/=n*1.0f;
        result[h] = sqrtf((sin[h]*sin[h]+cos[h]*cos[h]))/sqrtf(2.0);
    }
    free(sin);
    free(cos);
    return result;
}

// Generate test audio signal
void generate_test_audio(complex_t* signal, int n, float sample_rate) {
    // Generate a signal with multiple frequency components
    float f1 = 440.0;   // A4 note
    float f2 = 554.37;  // C#5 note
    float f3 = 659.25;  // E5 note
    
    for (int i = 0; i < n; i++) {
        float t = i / sample_rate;
        signal[i] = 0.5 * sin(TWO_PI * f1 * t) +
                   0.3 * sin(TWO_PI * f2 * t) +
                   0.2 * sin(TWO_PI * f3 * t) +
                   0.1 * ((float)rand() / RAND_MAX - 0.5);  // Add noise
    }
}

// Compute frequency for each FFT bin
float bin_to_frequency(int bin, int fft_size, float sample_rate) {
    return bin * sample_rate / fft_size;
}


void find_peaks(float* magnitude, int n, float sample_rate, 
                peak_t* peaks, int* num_peaks, int max_peaks) {
    *num_peaks = 0;
    float threshold = 0.1;  // Minimum magnitude threshold
    
    for (int i = 1; i < n/2 - 1 && *num_peaks < max_peaks; i++) {
        // Check if local maximum
        if (magnitude[i] > magnitude[i-1] && 
            magnitude[i] > magnitude[i+1] &&
            magnitude[i] > threshold) {
            
            peaks[*num_peaks].bin = i;
            peaks[*num_peaks].frequency = bin_to_frequency(i, n, sample_rate);
            peaks[*num_peaks].magnitude = magnitude[i];
            (*num_peaks)++;
        }
    }
    
    // Sort peaks by magnitude (simple bubble sort)
    for (int i = 0; i < *num_peaks - 1; i++) {
        for (int j = 0; j < *num_peaks - i - 1; j++) {
            if (peaks[j].magnitude < peaks[j+1].magnitude) {
                peak_t temp = peaks[j];
                peaks[j] = peaks[j+1];
                peaks[j+1] = temp;
            }
        }
    }
}

void generate_sin_dds(complex_t *signal, int n, float sample_freq, float *freq, int freq_num){
    for(int i=0;i<n;i++){
        for(int h=0;h<freq_num;h++){
            uint16_t tuning_word = (uint16_t)(i*freq[h]*LUT_SIZE/sample_freq)%LUT_SIZE;
            signal[i] += (float)lut[tuning_word];
        }
    }
}

// Display spectrum as ASCII art
void display_spectrum_ascii(float* magnitude, int n, float sample_rate) {
    int display_bins = 64;  // Number of bins to display
    if (display_bins > n/2) display_bins = n/2;
    
    // Find maximum magnitude for scaling
    float max_mag = 0;
    for (int i = 0; i < display_bins; i++) {
        if (magnitude[i] > max_mag) max_mag = magnitude[i];
    }
    
    printf("\nFrequency Spectrum:\n");
    printf("==================\n");
    
    // Display each frequency bin
    for (int i = 0; i < display_bins; i++) {
        float freq = bin_to_frequency(i, n, sample_rate);
        int bar_length = (int)(50 * magnitude[i] / max_mag);
        
        printf("%5.0f Hz |", freq);
        for (int j = 0; j < bar_length; j++) {
            printf("█");
        }
        printf(" %.3f\n", magnitude[i]);
    }
}

void display_dft(float* magnitude, int n, float sample_rate,float* frequencies, int freq_num){
    float max_mag = 0;
    for (int i = 0; i < freq_num; i++) {
        if (magnitude[i] > max_mag) max_mag = magnitude[i];
    }
    printf("\nFrequency Spectrum:\n");
    printf("==================\n");
    
    // Display each frequency bin
    for (int i = 0; i < freq_num; i++) {
        int idx = i%12;
        int exp = 1<<(i/12);
        float freq = frequencies[idx]*exp*(1.0f);
        int bar_length = (int)(50 * magnitude[i] / max_mag);
        
        printf("%5.0f Hz |", freq);
        for (int j = 0; j < bar_length; j++) {
            printf("█");
        }
        printf(" %.3f\n", magnitude[i]);
    }   
}

// Analyze audio spectrum
void analyze_audio_spectrum(complex_t* signal, int n, float sample_rate, 
                          const char* window_type) {
    // Apply window function
    if (strcmp(window_type, "hann") == 0) {
        apply_window_hann(signal, n);
    } else if (strcmp(window_type, "hamming") == 0) {
        apply_window_hamming(signal, n);
    } else if (strcmp(window_type, "blackman") == 0) {
        apply_window_blackman(signal, n);
    }
    
    // Compute FFT
    radix2_dit_fft(signal, n, FFT_FORWARD);
    
    // Compute magnitude spectrum
    float* magnitude = compute_magnitude(signal, n);
    
    // Find and display peaks
    peak_t peaks[10];
    int num_peaks;
    find_peaks(magnitude, n, sample_rate, peaks, &num_peaks, 10);
    
    printf("\nDetected Peaks:\n");
    printf("===============\n");
    for (int i = 0; i < num_peaks && i < 5; i++) {
        printf("%d. %.1f Hz (magnitude: %.3f)\n", 
               i+1, peaks[i].frequency, peaks[i].magnitude);
    }
    
    // Display spectrum
    display_spectrum_ascii(magnitude, n, sample_rate);
    
    free(magnitude);
}


