#include "Oscillator.h"
#include <cmath>

class OscillatorHarmonics {
public:
  OscillatorHarmonics();
  OscillatorHarmonics(float frequency, float fs, unsigned type,
                      unsigned numHarmonics = 3, unsigned harmonicType = all);
  ~OscillatorHarmonics(){};

  void setup(float frequency, float fs, unsigned int type, int numHarmonics = 3,
             unsigned harmonicType = all);

  float process();

  enum harmonic_type { all, even, uneven, numHarmonicTypes };

private:
  Oscillator oscillators[5];
  unsigned numHarmonics_;
  unsigned harmonicType_;
};
