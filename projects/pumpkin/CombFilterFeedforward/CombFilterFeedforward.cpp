#include "CombFilterFeedforward.h"

CombFilterFeedforward::CombFilterFeedforward(float fs, float delayLength, long maxDelayLength, float feedback) {
  _fs = fs;
  _delay = Delay(fs, delayLength, maxDelayLength);
  _feedback = feedback;
}

float CombFilterFeedforward::process(float input) {
  float previousValue = _delay.process(input, [&](float previousSample) -> float {
    return input;
  });

  return (previousValue * _feedback + input) * 0.5;
}

void CombFilterFeedforward::setDelayLength(float delayLength) {
  _delay.setDelayLength(delayLength);
}

void CombFilterFeedforward::setFeedback(float feedback) {
   _feedback = feedback;
}

CombFilterFeedforward::~CombFilterFeedforward() {}
