#pragma once
#include <vector>

using namespace std;

class Delay {
 private:
  vector<float> _samples;
  int long _writePosition;
  int long _delayLength;
  int long _maxDelayLength;
  float _fs;

 public:
  Delay(float fs=41000, float delayLength = 1, long maxDelayLength = 1);
  ~Delay();
  void setDelayLength(float delayLength);
  float process(float input, function<float(float previousSample)> writeDelaySample);
};
