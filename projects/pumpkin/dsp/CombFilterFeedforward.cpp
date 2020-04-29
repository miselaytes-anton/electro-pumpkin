#include "CombFilterFeedforward.h"

CombFilterFeedforward::CombFilterFeedforward(float fs, float delayLength, long maxDelayLength, float feedback) {
  _fs = fs;
  _delay = Delay(fs, delayLength, maxDelayLength);
  _feedback = feedback;
}

void CombFilterFeedforward::setDelayLength(float delayLength) {
  _delay.setDelayLength(delayLength);
}

void CombFilterFeedforward::setFeedback(float feedback) {
   _feedback = feedback;
}

CombFilterFeedforward::~CombFilterFeedforward() {}
