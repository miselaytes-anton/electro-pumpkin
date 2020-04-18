#pragma once
#include "../CombFilterFeedback/CombFilterFeedback.h"
#include "../CombFilterFeedforward/CombFilterFeedforward.h"

class AllPassFilter {
 private:
  CombFilterFeedforward _combFilterFeedforward;
  CombFilterFeedback _combFilterFeedback;
  float _delayLength;
  float _feedback;
  float _fs;

 public:
  AllPassFilter(float fs = 41000, float delayLength = 1, float feedback = 0.7);
  void setDelayLength(float delayLength);
  ~AllPassFilter();
  float process(float input);
};
