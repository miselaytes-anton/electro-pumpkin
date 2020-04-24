#include "Freeverb.h"
#include <cmath>

void Freeverb::setDelayTimes(float factor) {
  for (int i = 0; i < 8; i++) {
    fbcf[i]->setDelayLength(delayTimes[i] * factor);
  }
}

void Freeverb::setFeedback(float feedback) {
  for (int i = 0; i < 8; i++) {
    fbcf[i]->setFeedback(feedback);
  }
}

Freeverb::Freeverb(float fs, float delayFactor, float feedback) {
  _fs = fs;
  float damping = 2000;

  for (int i = 0; i < 8; i++) {
    fbcf[i] = new CombFilterFeedback(fs, delayTimes[i] * delayFactor,
                                     ceil(delayTimes[i] * delayFactor * 2), feedback);
    lowpass[i] = new OnePole(fs, damping);                             
  }

  for (int i = 0; i < 4; i++) {
    apf[i] = new AllPassFilter(_fs, allpassTimes[i], feedback);
  }
}

float Freeverb::process(float in) {
  float out = 0.0f;

  // comb filters in parallel
  for (int i = 0; i < 8; i++) {
    out += fbcf[i]->process(in, [&](float delayedSample) -> float {
      return lowpass[i]->process(delayedSample) * 0.5;
    });
  }

  // sequence of all pass filters
  for (int i = 0; i < 4; i++) {
    out = apf[i]->process(out);
  }

  return out * 2;
}
