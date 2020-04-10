#include "Delay.h"

Delay::Delay(float delayLength, float maxDelayLength, float feedback) {
	_samples.resize(maxDelayLength);
	_delayLength = delayLength;
	_maxDelayLength = maxDelayLength;
	_feedback = feedback;
	_writePosition = delayLength - 1;
	_readPosition = 0;
}

void Delay::setDelayLength(float delayLength) {
	_delayLength = delayLength;
}

float Delay::process(float input) { 
	float output = input + _samples[_readPosition] * _feedback;
	_samples[_writePosition] = input + _samples[_readPosition] * _feedback;
	_writePosition++;
	_readPosition++;
	if (_writePosition >= _delayLength){
		_writePosition = 0;
	}
	if (_readPosition >= _delayLength){
		_readPosition = 0;
	}

	return output; 
}

Delay::~Delay() {}
