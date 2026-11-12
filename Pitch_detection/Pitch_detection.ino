/*
   I2S stereo microphone (input) example
   Run using the Arduino Serial Plotter to see waveform.
   Released to the Public Domain by Earle F. Philhower, III

   For the Google AIY Voice Hat Microphone daughterboard, part
   of the Raspberry Pi AIY cardboard box, the I2S stereo pinout
   looking at the board top with the RPI logo on the left hand
   side:
            +--   ------------------------------------  --+
   left RPI | (1) GND (2) DIN (3) BCLK (4) LRCLK (5) 3.3V | AIY right
       logo +---------------------------------------------+ logo

   For an Adfruit I2S MEMS microphone (https://www.adafruit.com/product/3421),
   connect the pins as follows:

      DOUT -> GPIO0
      BCLK <- GPIO1
      LRCL <- GPIO2  # LRCLK = BCLK + 1
      GND <-> GND
      3V  <-> 3V3OUT

   The other idiosyncrasy of most modern MEMS microphones is that they
   require a minimum clock rate to wake up. For example, the SPH0645
   microphone needs at least 1MHz.
*/

#include <I2S.h>
#include "filter.h"
#include "pitch_detection.h"
#define AMP_THRESH 200000000
#define AMP_THRESH2 500000000
#define MIN_RATIO 10
#define SUSTAIN_HIGH 1.2
#define SUSTAIN_LOW 0.8
#define CORE1_BUFFERS 5

I2S i2s(INPUT);

Complex buffers[2][BUFFER_SIZE];

float E_curr;

bool set_flag = false;

float set_ratio = MIN_RATIO;

void setup() {
  Serial.begin(115200);
  i2s.setSysClk(SAMPLE_RATE);
  i2s.setDATA(0);
  i2s.setBCLK(1); // Note: LRCLK = BCLK + 1
  i2s.setBitsPerSample(16);
  i2s.setFrequency(SAMPLE_RATE);
  // NOTE: The following values are known to work with the Adafruit microphone:
  // i2s.setBitsPerSample(32);
  // i2s.setFrequency(16000);
  i2s.begin();

  uint16_t E_A_index=0;
  uint16_t E_B_index=0;
  uint8_t tmp =1;
  float E_A = 0;
  float E_B = 0;

  while (1) {
    if(tmp!=0){
      Serial.printf("Hello? %d\n\r", tmp);
      compute_E_first(&E_A,&E_A_index,&tmp);
    }
    else{
      int16_t l, r;
      i2s.read16(&l, &r);
      float re = Butterworth_filter(l*1.0f);
      // float re =l*1.0f;
      buffers[0][E_A_index].set(re*window_hamming(E_A_index),0.0);
      buffers[1][E_B_index].set(re*window_hamming(E_B_index),0.0);
      E_A_index++;
      E_B_index++;
      //Serial.printf("A: %d, B: %d \n\r",E_A_index,E_B_index);
      E_A += re*re;
      E_B += re*re;
      if(E_A_index>=BUFFER_SIZE){
        assing_reset(&E_A,&E_A_index,0);
      }
      if(E_B_index>=BUFFER_SIZE){
        assing_reset(&E_B,&E_B_index,1);
      }
    }
    // int16_t l,r;
    // i2s.read16(&l,&r);
    // Serial.printf("%d \n\r",l);
  }
}

void loop() {
}



void compute_E_first(float *E,uint16_t *index, uint8_t *tmp){
  int16_t l,r;
  Serial.printf("Hellov2?\n\r");
  for(uint16_t i=0;i<BUFFER_SIZE/2;i++){
    i2s.read16(&l, &r);
    float re = Butterworth_filter(l*1.0f);
    // float re =l*1.0f;
    buffers[0][i].set(re*window_hamming(i),0.0);
    *index = i;
    *E += re*re;
    Serial.printf("%f ", *E);
  }
  *tmp = 0;
  Serial.printf("\n\r Done with first loop (tmp = %d E = %f index = %d\n\r",*tmp,*E,*index);
}




void assing_reset(float *E,uint16_t *idx, uint8_t type){
  float E_ratio = *E/E_curr;
  //Energy  high enough to check for new sound
  if(*E>AMP_THRESH){
      if(set_flag == true){
        if(E_ratio > SUSTAIN_HIGH){
          //Serial.printf("Counter set\n\r");
          
        }
      }
      else{
        //checking if the ratio is high enough for the new sound
        if(E_ratio > MIN_RATIO){
          //Serial.printf("Counter set\n\r");
          
        }
        set_flag = true;
        //Serial.printf("Flag set\n\r");
      }
  }
  else{
    if(set_flag == true){
      set_flag = false;
      //Serial.printf("Flag unset\n\r");
    }
  }

  Serial.printf("%.1f %.4f \n\r", *E,fmod(E_ratio,10.0));

  E_curr = *E;
  *E = 0.0;
  *idx = 0;
}

void check_ratio(float *E){
  float E_ratio = *E/E_curr;
  Serial.printf("Checking new ratio\n\r");
  if(set_ratio == MIN_RATIO*1.0f){
    set_ratio = 1.5;
    Serial.printf("NEW RATIO SET\n\r");
  }
  else if(set_ratio != MIN_RATIO*1.0f && E_ratio<0.2){
    set_ratio = MIN_RATIO*1.0f;
  }
}
