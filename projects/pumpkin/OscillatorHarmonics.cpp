#include "OscillatorHarmonics.h"
#include <libraries/Oscillator/Oscillator.h>
#include <cmath>

OscillatorHarmonics::OscillatorHarmonics(){};
OscillatorHarmonics::OscillatorHarmonics(float frequency, float fs,
                                         unsigned int type, float initialPhase,
                                         unsigned int numHarmonics) {
  setup(frequency, fs, type, initialPhase, numHarmonics);
}

void OscillatorHarmonics::setup(float frequency, float fs, unsigned int type,
                                float initialPhase, int numHarmonics) {
  numHarmonics_ = numHarmonics;
  oscillators[0] = Oscillator{frequency, fs, type};
  for (unsigned int i = 1; i < numHarmonics_; i++) {
    oscillators[i] = Oscillator{frequency * (float(i) + 1), fs, type};
  }
}

float OscillatorHarmonics::process() {
  float out = 0;
  for (unsigned int i = 0; i < numHarmonics_; i++) {
    out += oscillators[i].process() / float(pow(2, i + 1));
  }
  return out;
}
