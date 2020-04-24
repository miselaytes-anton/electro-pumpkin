#include "Flanger.h"
#include <cmath>

Flanger::Flanger(float fs, float avgDelayLength, float lfoFreq, float depth) {
  _fs = fs;
  _avgDelayLength = avgDelayLength;
  _combFilter = CombFilterFeedforward(fs, avgDelayLength, ceil(avgDelayLength) * 2, depth);
  _lfo = Oscillator(fs, lfoFreq);
}

float Flanger::process(float input) {
  float maxDelaySwing = 1;
  float newDelayLength = _avgDelayLength * (1 + maxDelaySwing * _lfo.process());
  _combFilter.setDelayLength(newDelayLength);

  return _combFilter.process(input);
}

Flanger::~Flanger() {}
