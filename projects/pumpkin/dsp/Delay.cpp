#include "Delay.h"

using namespace std;

Delay::Delay(float fs, float delayLength, long maxDelayLength) {
  _fs = fs;
  _samples.resize(maxDelayLength * fs);
  _delayLength = delayLength * fs;
  _maxDelayLength = maxDelayLength * fs;
  _writePosition = 0;
}

void Delay::setDelayLength(float delayLength) {
  _delayLength = delayLength * _fs;
}

Delay::~Delay() {}
