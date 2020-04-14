#pragma once
#include "../Delay/Delay.h"

class CombFilterFeedback {
 private:
  Delay _delay;
  float _feedback;
  float _fs;

 public:
  CombFilterFeedback(float fs = 41000, float delayLength = 1, long maxDelayLength = 1, float feedback = 0);
  ~CombFilterFeedback();
  void setDelayLength(float delayLength);
  float process(float input);
};
