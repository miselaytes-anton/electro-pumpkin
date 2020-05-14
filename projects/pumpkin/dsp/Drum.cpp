#include "Drum.h"

Drum::Drum(float fs, float frequency) {
  _combFilter = CombFilterFeedback(fs, 1 / frequency, 1, 0.97f);
  _lowpass = Biquad{bq_type_lowpass, 1700 / fs, 0.707, 1};
}

Drum::~Drum() {}
