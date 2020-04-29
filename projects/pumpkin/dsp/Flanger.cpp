#include "Flanger.h"
#include <cmath>

Flanger::Flanger(float fs, float avgDelayLength, float lfoFreq, float depth) {
  _fs = fs;
  _avgDelayLength = avgDelayLength;
  _combFilter = CombFilterFeedforward(fs, avgDelayLength, ceil(avgDelayLength) * 2, depth);
  _lfo = Oscillator(fs, lfoFreq);
}

Flanger::~Flanger() {}
