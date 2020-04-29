#include "OscillatorHarmonics.h"
#include <cmath>

OscillatorHarmonics::OscillatorHarmonics(){};
OscillatorHarmonics::OscillatorHarmonics(float frequency, float fs,
                                         unsigned type, unsigned numHarmonics,
                                         unsigned harmonicType) {
  setup(frequency, fs, type, numHarmonics, harmonicType);
}

void OscillatorHarmonics::setup(float frequency, float fs, unsigned type,
                                int numHarmonics, unsigned harmonicType) {
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
