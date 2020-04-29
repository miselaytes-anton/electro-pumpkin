#include "CombFilterFeedback.h"

CombFilterFeedback::CombFilterFeedback(float fs, float delayLength,
                                       int long maxDelayLength,
                                       float feedback) {
  _fs = fs;
  _delay = Delay(fs, delayLength, maxDelayLength);
  _feedback = feedback;
}

void CombFilterFeedback::setDelayLength(float delayLength) {
  _delay.setDelayLength(delayLength);
}

void CombFilterFeedback::setFeedback(float feedback) { _feedback = feedback; }

CombFilterFeedback::~CombFilterFeedback() {}
