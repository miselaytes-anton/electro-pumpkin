#include "Delay.h"
#include <cmath>

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

float Delay::process(float input, function<float(float delayedSample, float inputSample)> writeDelaySample) {
  int long delayLengthCeil = ceil(_delayLength);
  float delta = delayLengthCeil - _delayLength;

  int long readPositionPrevious =
      _writePosition - delayLengthCeil < 0
          ? _maxDelayLength + _writePosition - delayLengthCeil
          : _writePosition - delayLengthCeil;

  float delayedSample = _samples[readPositionPrevious + 1] + delta * (_samples[readPositionPrevious] - _samples[readPositionPrevious + 1]);
		  
  _samples[_writePosition] = writeDelaySample(delayedSample, input);

  if (++_writePosition >= _maxDelayLength) {
    _writePosition = 0;
  }

  return delayedSample;
}

Delay::~Delay() {}
