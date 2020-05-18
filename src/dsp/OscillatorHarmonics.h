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

  inline float process() {
    float out = 0;
    for (unsigned i = 0; i < numHarmonics_; i++) {
      out += oscillators[i].process() / float(pow(2, i + 1));
    }
    return out;
  };

  enum harmonic_type { all, even, uneven, numHarmonicTypes };

private:
  Oscillator oscillators[5];
  unsigned numHarmonics_;
  unsigned harmonicType_;
};
