#pragma once
#include "../Delay/Delay.h"

class CombFilterFeedforward {
 private:
  Delay _delay;
  float _feedback;
  float _fs;

 public:
  CombFilterFeedforward(float fs = 41000, float delayLength = 1, long maxDelayLength = 1, float feedback = 0);
  ~CombFilterFeedforward();
  void setDelayLength(float delayLength);
  void setFeedback(float feedback);
  float process(float input);
};
