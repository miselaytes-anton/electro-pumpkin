#pragma once

#include "CombFilterFeedback.h"
#include "CombFilterFeedforward.h"
#include <functional>

class AllPassFilter {
private:
  CombFilterFeedforward _combFilterFeedforward;
  CombFilterFeedback _combFilterFeedback;
  float _delayLength;
  float _feedback;
  float _fs;

public:
  AllPassFilter(float fs = 41000.0f, float delayLength = 1.0f,
                float feedback = 0.7f);
  void setDelayLength(float delayLength);
  ~AllPassFilter();
  inline float process(float input) {
    return _combFilterFeedforward.process(_combFilterFeedback.process(input));
  };
};
