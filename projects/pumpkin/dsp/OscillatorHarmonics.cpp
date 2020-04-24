#include "OscillatorHarmonics.h"
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
  oscillators[0] = Oscillator{fs, frequency, type};

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
    oscillators[i] = Oscillator{fs, harmonicF, type};
  }
}

float OscillatorHarmonics::process() {
  float out = 0;
  for (unsigned i = 0; i < numHarmonics_; i++) {
    out += oscillators[i].process() / float(pow(2, i + 1));
  }
  return out;
}
