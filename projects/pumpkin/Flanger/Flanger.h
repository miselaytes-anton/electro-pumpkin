#pragma once
#include "../CombFilterFeedforward/CombFilterFeedforward.h"
#include "../Oscillator/Oscillator.h"

class Flanger {
 private:
  CombFilterFeedforward _combFilter;
  Oscillator _lfo;
  float _avgDelayLength;
  float _fs;

 public:
  Flanger(float fs = 41000, float avgDelayLength = 0.01f, float lfoFreq = 1, float depth = 1);
  ~Flanger();
  float process(float input);
};
