#include "wavformat.h"
#include "pitch_detection.h"
#include "audio_spectrum.h"

float fundamental_freq[] = {65.41,69.30,73.42,77.78,
                            82.41,87.31,92.50,98.00,
                            103.83,110.00,116.54,123.47};

int freq_number = 49;

float compute_energy(complex_t* signal, int n){
    float energy = 0;
    for(int i=0;i<n;i++){
        energy +=pow(signal[i],2.0);
    }
    return energy;
}

bool check_new_pitch(float *pitches, float *curr_pitches, int idx){
    if(pitches[idx]!=curr_pitches[idx]){
        return 1;
    }
    return 0;

}
const char* methods[] = {"Maximum Peak", "HPS", "Autocorrelation"};
 


///


///
void display_current_pitch_wav(float energy,float *pitches, float confidence, int num_frame, const char * method){
    int idx = -1;
    for(int i; i< 3;i++){
        if(strcmp(method,methods[i]) == 0){
            idx = i;
            printf("Frame:%d\n",num_frame);
            printf("Energy: %.1f\n",energy);
            printf("Method: %s\n",methods[idx]);
            printf("Detected pitch: %.2f Hz\n", pitches[idx]);
            printf("Musical note: %s\n", frequency_to_note_name(pitches[idx]));
            printf("Confidence: %.1f%%\n\n", confidence * 100);
            break;
        }
    }
    if(idx == -1){
        printf("Cannot find method: %s\n\n",method);
    }

}
void display_spectrum_ascii_v2(float* magnitude, int k, float *fundamentals) {
    
    // Find maximum magnitude for scaling
    float max_mag = 0;
    for (int i = 0; i < k; i++) {
        if (magnitude[i] > max_mag) max_mag = magnitude[i];
    }
    
    printf("\nFrequency Spectrum:\n");
    printf("==================\n");
    
    // Display each frequency bin
    for (int i = 0; i < k; i++) {
        float freq = fundamentals[i%12] * pow(2,(int)i/12);
        int bar_length = (int)(50 * magnitude[i] / max_mag);
        
        printf("%5.0f Hz |", freq);
        for (int j = 0; j < bar_length; j++) {
            printf("█");
        }
        printf(" %.3f\n", magnitude[i]);
    }
}

void analyze_wav_file(sound_t sound, int n, float sample_rate){
    static uint32_t sound_position = 0; //Variable to iterate over the samples
    float *curr_pitches = calloc(3,sizeof(float)); // current pitches (estimated by each method)
    float curr_energy =0.0;    //Energy of last analyzed signal frame
    int num_frame = 0;
    int idx = 0;
    // Main loop
    while(sound_position < sound.samples){
        complex_t* signal = allocate_complex_array(n);
        for(int i=0; i < n;i++){
            if(sound_position < sound.samples){
                signal[i] = (float)sound.data[sound_position++] + 0*I;
            }
            else{
                signal[i] = 0.0 + 0*I;
            }
        }
        num_frame++;
        float energy = compute_energy(signal,n);
        float energy_ratio = curr_energy/energy;
        
        //energy of next frames is rising == new note
            apply_window_hann(signal, n);
            // memcpy(spectrum, signal, n * sizeof(complex_t));
            complex_t *spectrum = compute_dft(signal,n,sample_rate,fundamental_freq,freq_number);
            float *magnitude = compute_magnitude(spectrum,freq_number);
            display_dft(magnitude,n,sample_rate,fundamental_freq,freq_number);
            free_complex_array(spectrum);    
        // if(energy_ratio < 1){
        //     apply_window_hann(signal, n);
        //     complex_t* spectrum = allocate_complex_array(n);
        //     //memcpy(spectrum, signal, n * sizeof(complex_t));
        //     spectrum = compute_ndft(signal,n,fundamental_freq,freq_number);
        //     //radix2_dit_fft(spectrum, n, FFT_FORWARD);
        //     float *pitches = calloc(3,sizeof(float));
        //     // Method 1: Simple Maximum Peak
        //     // pitches[0] = detect_pitch_peak(spectrum,n,sample_rate);
        //     pitches[0] = detect_pitch_peak_v2(spectrum, freq_number, fundamental_freq);
        //     // // Method 2: HPS
        //     // pitches[1] = detect_pitch_hps(spectrum, n, sample_rate, 3);
        
        //     // // Method 3: Autocorrelation
        //     // pitches[2] = detect_pitch_autocorr(signal, n, sample_rate);
        //     // float avg_pitch = (pitches[0] + pitches[1] + pitches[1]) / 3;
        //     // float variance = pow(pitches[0] - avg_pitch, 2) + 
        //     //         pow(pitches[1] - avg_pitch, 2) + 
        //     //         pow(pitches[2] - avg_pitch, 2);
        //     // variance /= 3;

        //     // float confidence = 1.0 / (1.0 + sqrt(variance) / avg_pitch);
        //     // // if(check_new_pitch(pitches,curr_pitches,idx)){
        //     // //     for(int i=0;i<3;i++){
        //     // //         curr_pitches[i] = pitches[i];
        //     // //     }
        //     // //     display_current_pitch_wav(curr_pitches,confidence,num_frame, method);
        //     // // }
        //     // display_current_pitch_wav(energy_ratio,pitches,confidence,num_frame, method);
        //     printf("Frame:%d\n",num_frame);
        //     printf("Energy: %.1f\n",energy_ratio);
        //     printf("Method: %s\n",methods[idx]);
        //     printf("Detected pitch: %.2f Hz\n", pitches[idx]);
        //     printf("Musical note: %s\n", frequency_to_note_name(pitches[idx]));
        //     free_complex_array(spectrum);
        // }
        curr_energy = energy;
        free_complex_array(signal);
    }
}
// Main demonstration
int main() {
    printf("Music Pitch Detection using FFT\n");
    printf("================================\n\n");
    
    float sample_rate = 44100;
    int n = 10000;

    complex_t *signal = allocate_complex_array(n);
    float freq[] = {40.0,400.0,800.0,1200.0}; 
    generate_sin_dds(signal,n,sample_rate,freq,3);
    complex_t *filtered = allocate_complex_array(n);
    filterloop(signal,filtered,n);
    apply_window_hamming(signal,n);
    apply_window_hamming(filtered,n);
    complex_t *spectrum = compute_dft(signal,n,sample_rate,fundamental_freq,freq_number);
    complex_t *spectrum2 = compute_dft(filtered,n,sample_rate,fundamental_freq,freq_number);
    float *magnitude = compute_magnitude(spectrum,n);
    float *magnitude2 = compute_magnitude(spectrum2,n);
    display_dft(magnitude,n,sample_rate,fundamental_freq,freq_number);
    display_dft(magnitude2,n,sample_rate,fundamental_freq,freq_number);
    free(signal);

    // printf("\nWav file analyze test\n\n");
    // sound_t sound;
	// if(!LoadWav("wav/guitar-pack-a-string.wav", &sound)) {
	// 	PRINT_ERROR("Failed to load cheers.wav");
	// }
    // else{
    //     printf("Wav file loaded succesfully\n");
    // }
    // analyze_wav_file(sound,n,sample_rate);
    return 0;
    // FILE *file;
    // FILE *file2;

    // complex_t *signal = allocate_complex_array(n);
    // float frequencies[] = {330.0,660.0,990.0,1200.0,1500.0,2000.0};
    // generate_sin_dds(signal,n,sample_rate,frequencies,6);
    // complex_t* filter = allocate_complex_array(n);
    // memcpy(filter, signal, n * sizeof(complex_t));
    // filterloop(signal,filter,n);
    // file = fopen("data/filtr_test.txt","w");
    // file2 = fopen("data/sin_test.txt","w");
    // if(file == NULL || file2 == NULL){
    //     perror("Couldn't open file\n");
    //     exit(-1);
    // }
    // for(int i=0;i<n;i++){
    //     if(fprintf(file,"%f\n",creal(filter[i]))<0 || fprintf(file2,"%f\n",creal(signal[i]))<0){
    //         printf("Something gone wrong...\n");
    //         exit(-1);
    //     }
    // }
    // printf("Success?...\n");
    // fclose(file);
}
