#include "CombFilterFeedforward.h"

CombFilterFeedforward::CombFilterFeedforward(float fs, float delayLength, long maxDelayLength, float feedback) {
  _fs = fs;
  _delay = Delay(fs, delayLength, maxDelayLength);
  _feedback = feedback;
}

float CombFilterFeedforward::process(float input) {
  float previousValue = _delay.process(input);

  return (input * _feedback + previousValue);
}

void CombFilterFeedforward::setDelayLength(float delayLength) {
  _delay.setDelayLength(delayLength);
}

void CombFilterFeedforward::setFeedback(float feedback) {
   _feedback = feedback;
}

CombFilterFeedforward::~CombFilterFeedforward() {}
