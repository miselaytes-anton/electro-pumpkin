#pragma once
#include <functional>

#include "Delay.h"

template <typename T>
class Identity {
public:
    Identity() {}
    T operator () (T value) const {
        return value;
    }
};

class CombFilterFeedback {
private:
  Delay _delay;
  float _feedback;
  float _fs;

public:
  CombFilterFeedback();
  CombFilterFeedback(float fs = 41000, float delayLength = 1,
                     long maxDelayLength = 2, float feedback = 0);
  ~CombFilterFeedback();
  void setDelayLength(float delayLength);
  void setFeedback(float feedback);
  float process(float input,
                function<float(float delayedSample)> processDelayedSignal =
                    Identity<float>());
};
