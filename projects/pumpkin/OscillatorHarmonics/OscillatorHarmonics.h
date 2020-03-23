#include <libraries/Oscillator/Oscillator.h>
#include <cmath>

class OscillatorHarmonics {
 public:
  OscillatorHarmonics();
  OscillatorHarmonics(float frequency, float fs, unsigned int type,
                      float initialPhase = 0, unsigned int numHarmonics = 3);
  ~OscillatorHarmonics(){};

  void setup(float frequency, float fs, unsigned int type,
             float initialPhase = 0, int numHarmonics = 3);

  float process();

 private:
  Oscillator oscillators[5];
  unsigned int numHarmonics_;
};
