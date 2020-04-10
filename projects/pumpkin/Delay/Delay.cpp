#include "Delay.h"

Delay::Delay(int long delayLength, int long maxDelayLength,
             float feedback) {
  _samples.resize(maxDelayLength);
  _delayLength = delayLength;
  _maxDelayLength = maxDelayLength;
  _feedback = feedback;
  _writePosition = 0;
}

void Delay::setDelayLength(int long delayLength) {
  _delayLength = delayLength;
}

float Delay::process(float input) {
  int long readPosition =
      _writePosition - _delayLength < 0
          ? _maxDelayLength + _writePosition - _delayLength
          : _writePosition - _delayLength;
		  
  float output = (input + _samples[readPosition] * _feedback) * 0.5;
  _samples[_writePosition] = output;
  _writePosition++;
  if (_writePosition >= _maxDelayLength) {
    _writePosition = 0;
  }

  return output;
}

Delay::~Delay() {}
