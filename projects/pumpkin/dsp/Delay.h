#pragma once

#include <cmath>
#include <functional>
#include <vector>

using namespace std;

template <typename T> class DefaultWriteDelaySample {
public:
  DefaultWriteDelaySample() {}
  T operator()(T delayedSample, T inputSample) const { return inputSample; }
};

class Delay {
private:
  vector<float> _samples;
  int long _writePosition;
  int long _delayLength;
  int long _maxDelayLength;
  float _fs;
  std::function<float(float)> writeDelaySample;

public:
  Delay(float fs = 41000, float delayLength = 1, long maxDelayLength = 2);
  ~Delay();
  void setDelayLength(float delayLength);
  inline float process(
      float input,
      function<float(float delayedSample, float inputSample)> writeDelaySample =
          DefaultWriteDelaySample<float>()) {
    int long delayLengthCeil = ceil(_delayLength);
    float delta = delayLengthCeil - _delayLength;

    int long readPositionPrevious =
        _writePosition - delayLengthCeil < 0
            ? _maxDelayLength + _writePosition - delayLengthCeil
            : _writePosition - delayLengthCeil;

    float delayedSample = _samples[readPositionPrevious + 1] +
                          delta * (_samples[readPositionPrevious] -
                                   _samples[readPositionPrevious + 1]);

    _samples[_writePosition] = writeDelaySample(delayedSample, input);

    if (++_writePosition >= _maxDelayLength) {
      _writePosition = 0;
    }

    return delayedSample;
  };
};
