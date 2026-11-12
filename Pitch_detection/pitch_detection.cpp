#include "pitch_detection.h"

static  float fundamental_freq[] = {65.41,69.30,73.42,77.78,
                            82.41,87.31,92.50,98.00,
                            103.83,110.00,116.54,123.47};

 uint8_t notes_num = 49;

 musical_note_t notes[] = {
    {"C2", 65.41}, {"C#2", 69.30}, {"D2", 73.42}, {"D#2", 77.78},
    {"E2", 82.41}, {"F2", 87.31}, {"F#2", 92.50}, {"G2", 98.00},
    {"G#2", 103.83}, {"A2", 110.00}, {"A#2", 116.54}, {"B2", 123.47},
    {"C3", 130.81}, {"C#3", 138.59}, {"D3", 146.83}, {"D#3", 155.56},
    {"E3", 164.81}, {"F3", 174.61}, {"F#3", 185.00}, {"G3", 196.00},
    {"G#3", 207.65}, {"A3", 220.00}, {"A#3", 233.08}, {"B3", 246.94},
    {"C4", 261.63}, {"C#4", 277.18}, {"D4", 293.66}, {"D#4", 311.13},
    {"E4", 329.63}, {"F4", 349.23}, {"F#4", 369.99}, {"G4", 392.00},
    {"G#4", 415.30}, {"A4", 440.00}, {"A#4", 466.16}, {"B4", 493.88},
    {"C5", 523.25}, {"C#5", 554.37}, {"D5", 587.33}, {"D#5", 622.25},
    {"E5", 659.25}, {"F5", 698.46}, {"F#5", 739.99}, {"G5", 783.99},
    {"G#5", 830.61}, {"A5", 880.00}, {"A#5", 932.33}, {"B5", 987.77},
    {"C6", 1046.50}
};
const char* frequency_to_note_name(float freq) {
    int closest_idx = 0;
    float min_cents = 1200;  // Max cents difference
    
    for (int i = 0; i < notes_num; i++) {
        float cents = 1200 * log2(freq / notes[i].frequency);
        if (fabs(cents) < fabs(min_cents)) {
            min_cents = cents;
            closest_idx = i;
        }
    }
    
    static char result[32];
    if (fabs(min_cents) < 1) {
        snprintf(result, sizeof(result), "%s (in tune)", notes[closest_idx].name);
    } else {
        snprintf(result, sizeof(result), "%s (%+.0f cents)", 
                notes[closest_idx].name, min_cents);
    }
    
    return result;
}

float detect_pitch_peak(float* magnitude,  uint8_t notes_num){
    float peaks[10];
    uint8_t peaks_idx[10];
    uint8_t tmp = 0;
    float max_mag = 0;
    uint8_t freq_idx = 0;
    
    for (uint8_t i = 0; i < notes_num; i++) {
        if(magnitude[i] > 5.0 && tmp < 10){
            peaks_idx[tmp] = i;
            peaks[tmp++] = magnitude[i];
            //Serial.printf("Idx: %d  peak: %f\n\r",peaks_idx[tmp-1],peaks[tmp-1]);
            if (magnitude[i] > max_mag) {
                max_mag = magnitude[i];
                freq_idx = i;
            }
        }
    }
    float potential = freq_idx;
    for(uint8_t i = 0; i < tmp; i++){
        if((peaks[i]/max_mag) > 0.15){
            float tmp2 = notes[freq_idx].frequency/notes[peaks_idx[i]].frequency;
            float diff = tmp2 - (int)tmp2;
            //Serial.printf("DIFF: %f\n\r",diff);
            if((diff > 0.95 || diff < 0.05) && peaks_idx[i] != freq_idx){
                //Serial.printf("To sie dzieje\n\r");
                potential=peaks_idx[i];
            }
        }
    }
    freq_idx = potential;
    return notes[freq_idx].frequency;
}

float detect_pitch_hps(float* magnitude, uint8_t notes_num, uint8_t harmonics){
    float* hps = (float*)malloc((notes_num/2 + 1) * sizeof(float));
    
    // Initialize HPS with original spectrum
    for (uint8_t i = 0; i <= notes_num/2; i++) {
        hps[i] = magnitude[i];
    }
    
    // Multiply with downsampled versions
    for (uint8_t h = 2; h <= harmonics; h++) {
        for (uint8_t i = 0; i <= notes_num/(2*h); i++) {
            hps[i] *= magnitude[i * h];
        }
    }
    
    // Find peak in HPS    
    float max_hps = 0;
    uint8_t freq_idx = 0;
    
    for (uint8_t i = 0; i <= notes_num/2; i++) {
        if (hps[i] > max_hps) {
            max_hps = hps[i];
            freq_idx = i;
        }
    }
    
    free(hps);
    
    return notes[freq_idx].frequency;
}

float detect_pitch_fft(Complex *spectrum) {
    float* magnitude = compute_magnitude(spectrum, BUFFER_SIZE);
    
    // Find peak in reasonable frequency range (80-2000 Hz)
    int min_bin = (int)(80 * BUFFER_SIZE / SAMPLE_RATE);
    int max_bin = (int)(2000 * BUFFER_SIZE / SAMPLE_RATE);
    if (max_bin > BUFFER_SIZE/2) max_bin = BUFFER_SIZE/2;
    
    float max_mag = 0;
    int peak_bin = 0;
    
    for (int i = min_bin; i < max_bin; i++) {
        if (magnitude[i] > max_mag) {
            max_mag = magnitude[i];
            peak_bin = i;
        }
    }
    
    // Quadratic interpolation for more accurate frequency
    if (peak_bin > 0 && peak_bin < BUFFER_SIZE/2 - 1) {
        float y1 = magnitude[peak_bin - 1];
        float y2 = magnitude[peak_bin];
        float y3 = magnitude[peak_bin + 1];
        
        float delta = 0.5 * (y1 - y3) / (y1 - 2*y2 + y3);
        peak_bin += delta;
    }
    
    free(magnitude);
    
    return peak_bin * SAMPLE_RATE / BUFFER_SIZE;
}

void check_new_sound(Complex *buffer){
    Complex *spectrum = (Complex*)malloc(notes_num*sizeof(Complex));
    spectrum = compute_dft(buffer, BUFFER_SIZE, SAMPLE_RATE, fundamental_freq, notes_num);
    float* magnitude = compute_magnitude(spectrum, notes_num);
    float peak = detect_pitch_peak(magnitude,notes_num);
    // float hps = detect_pitch_hps(magnitude,notes_num,6);

    Serial.printf("Detected pitch (Peak): %.2f Hz\n", peak);
    Serial.printf("Musical note: %s\n", frequency_to_note_name(peak));
    // Serial.printf("Detected pitch (HPS): %.2f Hz\n", hps);
    // Serial.printf("Musical note: %s\n", frequency_to_note_name(hps));
    //display_dft(magnitude,fundamental_freq,notes_num);
    free(magnitude);
    free(spectrum);
}

void check_sound_fft(Complex *buffer){
    Complex *spectrum = allocate_complex_array(BUFFER_SIZE);
    memcpy(spectrum, buffer, BUFFER_SIZE * sizeof(Complex));
    radix2_dit_fft(spectrum, BUFFER_SIZE, FFT_FORWARD);
    float* magnitude = compute_magnitude(spectrum, notes_num);
    float peak = detect_pitch_peak(magnitude,notes_num);

    Serial.printf("Detected pitch (Peak): %.2f Hz\n", peak);
    Serial.printf("Musical note: %s\n", frequency_to_note_name(peak));

    free(magnitude);
    free(spectrum);
}