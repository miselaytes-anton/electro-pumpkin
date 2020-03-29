/***** Freeverb.h *****/
//https://github.com/djzielin/Freeverb-test

#include "allpass_filter.h"
#include "feedback_comb_filter.h"

// reimplimentation of freeverb algorithm
class Freeverb {
 private:
  float sampleRate;

  float delay_times[8] = {1116.0f / 44100.0f, 1188.0f / 44100.0f,
                          1277.0f / 44100.0f, 1356.0f / 44100.0f,
                          1422.0f / 44100.0f, 1491.0f / 44100.0f,
                          1557.0f / 44100.0f, 1617.0f / 44100.0f};

  float allpass_times[4] = {225.0f / 44100.0f, 556.0f / 44100.0f,
                            441.0f / 44100.0f, 341.0f / 44100.0f};

  feedback_comb_filter *fbcf[8];
  allpass_filter *apf[4];

 public:
  void set_delay_times(float factor);
  void set_feedback(float val);
  Freeverb(float sample_rate);
  float tick(float in);
};