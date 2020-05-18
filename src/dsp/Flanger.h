#pragma once

#include "CombFilterFeedforward.h"
#include "Oscillator.h"

class Flanger {
private:
  CombFilterFeedforward _combFilter;
  Oscillator _lfo;
  float _avgDelayLength;
  float _fs;

public:
  Flanger(float fs = 41000, float avgDelayLength = 0.01f, float lfoFreq = 1,
          float depth = 1);
  ~Flanger();
  inline float process(float input) {
    float maxDelaySwing = 1;
    float newDelayLength =
        _avgDelayLength * (1 + maxDelaySwing * _lfo.process());
    _combFilter.setDelayLength(newDelayLength);

    return _combFilter.process(input);
  };
};
