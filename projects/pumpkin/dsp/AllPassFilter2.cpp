#include "AllPassFilter2.h"

AllPassFilter2::AllPassFilter2(float fs, float delayLength, int long maxDelayLength, float feedback) {
  _fs = fs;
  _delay = Delay(fs, delayLength, maxDelayLength);
  _feedback = feedback;
  _delay2 = Delay(fs, delayLength, maxDelayLength);
}

float AllPassFilter2::process(float input) {
  float previousX = _delay.process(input, [&](float previousSample) -> float {return input;});
  float previousY = _delay2.process(input, [&](float previousSample) -> float {
    return (_feedback * input + previousX - _feedback * previousSample);
  });
  
  return (_feedback * input + previousX - _feedback * previousY) * 0.5;  
}

void AllPassFilter2::setDelayLength(float delayLength) {
  _delay.setDelayLength(delayLength);
}

void AllPassFilter2::setFeedback(float feedback) {
   _feedback = feedback;
}

AllPassFilter2::~AllPassFilter2() {}
