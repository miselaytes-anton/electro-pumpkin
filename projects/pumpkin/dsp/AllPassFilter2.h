#pragma once
#include "Delay.h"

class AllPassFilter2 {
 private:
  Delay _delay;
  float _feedback;
  float _fs;
  Delay _delay2;

 public:
  AllPassFilter2(float fs = 41000, float delayLength = 1,
                     long maxDelayLength = 1, float feedback = 0);
  ~AllPassFilter2();
  void setDelayLength(float delayLength);
  void setFeedback(float feedback);
  float process(float input);
};
