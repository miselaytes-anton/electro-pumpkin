#include "AllPassFilter.h"
#include <cmath>

AllPassFilter::AllPassFilter(float fs, float delayLength,
                             float feedback) {
  _fs = fs;
  _delayLength = delayLength;
  _feedback = feedback;
  _combFilterFeedforward =
      CombFilterFeedforward(fs, _delayLength, ceil(_delayLength * 2), _feedback);
  _combFilterFeedback =
      CombFilterFeedback(fs, _delayLength, ceil(_delayLength * 2), -_feedback);
}

void AllPassFilter::setDelayLength(float delayLength) {
  _combFilterFeedforward.setDelayLength(delayLength);
  _combFilterFeedback.setDelayLength(delayLength);
}

float AllPassFilter::process(float input) {
  return _combFilterFeedforward.process(_combFilterFeedback.process(input));
}

AllPassFilter::~AllPassFilter() {}
