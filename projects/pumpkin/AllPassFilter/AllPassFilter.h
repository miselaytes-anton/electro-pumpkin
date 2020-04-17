#pragma once
#include "../CombFilterFeedforward/CombFilterFeedforward.h"
#include "../CombFilterFeedback/CombFilterFeedback.h"

class AllPassFilter {
 private:
  CombFilterFeedforward _combFilterFeedforward;
  CombFilterFeedback _combFilterFeedback;
  float _delayLength;
  float _feedback;
  float _fs;
  void setDelayLength(float delayLength);

 public:
  AllPassFilter(float fs = 41000, float delayLength = 1, float feedback = 0.7);
  ~AllPassFilter();
  float process(float input);
};
