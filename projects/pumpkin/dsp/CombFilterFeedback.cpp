#include "CombFilterFeedback.h"

CombFilterFeedback::CombFilterFeedback(float fs, float delayLength,
                                       int long maxDelayLength,
                                       float feedback) {
  _fs = fs;
  _delay = Delay(fs, delayLength, maxDelayLength);
  _feedback = feedback;
}

float CombFilterFeedback::processInput(float inputSample, float delayedSample,
                                       float feedback) {
  return (inputSample - delayedSample * feedback) * 0.5;
}

float CombFilterFeedback::process(float input,
                                  function<float(float)> processDelayedSignal) {
  return _delay.process(input, [&](float previousSample) -> float {
    return processInput(input, processDelayedSignal(previousSample), _feedback);
  });
}

void CombFilterFeedback::setDelayLength(float delayLength) {
  _delay.setDelayLength(delayLength);
}

void CombFilterFeedback::setFeedback(float feedback) { _feedback = feedback; }

CombFilterFeedback::~CombFilterFeedback() {}
