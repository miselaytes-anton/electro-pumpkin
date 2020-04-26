#pragma once

#include <vector>
#include <functional>

using namespace std;

template <typename T>
class DefaultWriteDelaySample {
public:
    DefaultWriteDelaySample() {}
    T operator () (T delayedSample, T inputSample) const {
        return inputSample;
    }
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
  float process(float input,
                function<float(float delayedSample, float inputSample)> writeDelaySample = DefaultWriteDelaySample<float>());
};