#include "Delay.h"
#include <cmath>

using namespace std;

Delay::Delay(float delayLength, int long maxDelayLength,
             float feedback) {
  _samples.resize(maxDelayLength);
  _delayLength = delayLength;
  _maxDelayLength = maxDelayLength;
  _feedback = feedback;
  _writePosition = 0;
}

void Delay::setDelayLength(float delayLength) {
  _delayLength = delayLength;
}

float Delay::process(float input) {
  int long delayLengthCeil = ceil(_delayLength);
  float delta = delayLengthCeil - _delayLength;

  int long readPositionPrevious =
      _writePosition - delayLengthCeil < 0
          ? _maxDelayLength + _writePosition - delayLengthCeil
          : _writePosition - delayLengthCeil;

  float previousValue = _samples[readPositionPrevious + 1] + delta * (_samples[readPositionPrevious] - _samples[readPositionPrevious + 1]);
		  
  _samples[_writePosition] = (input + _feedback * previousValue) * 0.5;

  if (++_writePosition >= _maxDelayLength) {
    _writePosition = 0;
  }

  return _samples[_writePosition];
}

Delay::~Delay() {}


/**
 * feedback comb filter: 
 *    (input, previousValue, feedback) => (input + _feedback * previousValue) * 0.5, 
 *    (input, previousValue, feedback) => (input + _feedback * previousValue) * 0.5, 
 * feedforward comb filter: 
 *    (input, previousValue, feedback) => input,
 *    (input, previousValue, feedback) => (input + feedback * previousValue) * 0.5
 * flanger:
 *    delayLength = avgDelayLength * (1 + maxDelaySwing * sine.tick()) * samplingRate;
 *    (input, previousValue, feedback) => input,
 *    (input, previousValue, feedback) => (input + feedback * previousValue) * 0.5
**/
