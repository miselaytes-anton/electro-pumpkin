#pragma once

#include "Biquad.h"
#include "CombFilterFeedback.h"

class Drum {
private:
  CombFilterFeedback _combFilter;
  Biquad _lowpass;

public:
  Drum();
  Drum(float fs, float frequency);
  ~Drum();
  inline float process(float input) {
    return _combFilter.process(input, [&](float delayedSample) -> float {
      return _lowpass.process(delayedSample);
    });
  };
  void setFrequency(float frequency);
};
