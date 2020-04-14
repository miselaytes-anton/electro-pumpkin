#include "CombFilterFeedback.h"

CombFilterFeedback::CombFilterFeedback(float fs, float delayLength, int long maxDelayLength, float feedback) {
  _fs = fs;
  _delay = Delay(fs, delayLength, maxDelayLength);
  _feedback = feedback;
}

float CombFilterFeedback::process(float input) {
  return _delay.process(input, [&](float previousSample) -> float {
    return (previousSample * _feedback + input) * 0.5;
  });
}

void CombFilterFeedback::setDelayLength(float delayLength) {
  _delay.setDelayLength(delayLength);
}

CombFilterFeedback::~CombFilterFeedback() {}
