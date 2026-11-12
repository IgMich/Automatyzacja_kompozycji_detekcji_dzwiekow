#ifndef PITCH_DETECTION_H
#define PITCH_DETECTION_H
#include "signal_analyzing.h"

typedef struct {
    const char* name;
    float frequency;
} musical_note_t;

const char* frequency_to_note_name(float freq);
float detect_pitch_peak(float* magnitude,  uint8_t notes_num);
float detect_pitch_hps(float* magnitude, uint8_t notes_num, uint8_t harmonics);
float detect_pitch_fft(Complex *spectrum);
void check_new_sound(Complex *buffer);
void check_sound_fft(Complex *buffer);
#endif