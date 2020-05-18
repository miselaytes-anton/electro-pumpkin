#pragma once
#include "Delay.h"

class CombFilterFeedforward {
private:
  Delay _delay;
  float _feedback;
  float _fs;

public:
  CombFilterFeedforward(float fs = 41000, float delayLength = 1,
                        long maxDelayLength = 1, float feedback = 0);
  ~CombFilterFeedforward();
  void setDelayLength(float delayLength);
  void setFeedback(float feedback);
  inline float process(float input) {
    float previousValue = _delay.process(input);

    return (input * _feedback + previousValue);
  };
};
