#include "CombFilterFeedback.h"

CombFilterFeedback::CombFilterFeedback(){};
CombFilterFeedback::CombFilterFeedback(float fs, float delayLength,
                                       int long maxDelayLength,
                                       float feedback) {
  _fs = fs;
  _delay = Delay(fs, delayLength, maxDelayLength);
  _feedback = feedback;
}

float CombFilterFeedback::process(float inputSample,
                                  function<float(float)> processDelayedSignal) {
  float out;
  _delay.process(
      inputSample, [&](float delayedSample, float inputSample) -> float {
        out = (inputSample - processDelayedSignal(delayedSample) * _feedback) *
              0.5;
        return out;
      });
  return out;
}

void CombFilterFeedback::setDelayLength(float delayLength) {
  _delay.setDelayLength(delayLength);
}

void CombFilterFeedback::setFeedback(float feedback) { _feedback = feedback; }

CombFilterFeedback::~CombFilterFeedback() {}
