#pragma once
#include "../Delay/Delay.h"

class CombFilterFeedback {
 private:
  Delay _delay;
  OnePole lowpass;
  float _feedback;
  float _fs;

 public:
  CombFilterFeedback(float fs = 41000, float delayLength = 1, long maxDelayLength = 1, float feedback = 0);
  ~CombFilterFeedback();
  void setDelayLength(float delayLength);
  void setFeedback(float feedback);
  float process(float input);
};
