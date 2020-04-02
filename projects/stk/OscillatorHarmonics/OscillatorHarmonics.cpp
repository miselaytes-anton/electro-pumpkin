#include "OscillatorHarmonics.h"

#include <stk/BlitSaw.h>
#include <stk/BlitSquare.h>
#include <stk/SineWave.h>

#include <cmath>

OscillatorHarmonics::OscillatorHarmonics(){};
OscillatorHarmonics::OscillatorHarmonics(float frequency, float fs,
                                         unsigned type, float initialPhase,
                                         unsigned numHarmonics,
                                         unsigned harmonicType) {
  setup(frequency, fs, type, initialPhase, numHarmonics, harmonicType);
}

void OscillatorHarmonics::setup(float frequency, float fs, unsigned type,
                                float initialPhase, int numHarmonics,
                                unsigned harmonicType) {
  numHarmonics_ = numHarmonics;
  harmonicType_ = harmonicType;
  switch (type) {
    case sine:
      for (unsigned i = 0; i < numHarmonics_; i++) {
        oscillators[i] = &SineWave{};
      }
      break;
    case saw:
      for (unsigned i = 0; i < numHarmonics_; i++) {
        oscillators[i] = &BlitSaw{};
      }
      break;
    case square:
      for (unsigned i = 0; i < numHarmonics_; i++) {
        oscillators[i] = &BlitSquare{};
      }
      break;
  }

  for (unsigned i = 1; i < numHarmonics_; i++) {
    float harmonicF;
    switch (harmonicType_) {
      case all:
        harmonicF = frequency * (float(i) + 1);
        break;
      case even:
        harmonicF = frequency * (float(i) * 2);
        break;
      case uneven:
        harmonicF = frequency * (float(i) * 2 + 1);
        break;
    }
    oscillators[i] = Oscillator{harmonicF, fs, type};
  }
}

float OscillatorHarmonics::process() {
  float out = 0;
  for (unsigned i = 0; i < numHarmonics_; i++) {
    out += oscillators[i].process() / float(pow(2, i + 1));
  }
  return out;
}
