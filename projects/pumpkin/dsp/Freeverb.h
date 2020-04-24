#pragma once
#include "AllPassFilter.h"
#include "CombFilterFeedback.h"

class Freeverb {
 private:
  float _fs;

  float delayTimes[8] = {1116.0f / 44100.0f, 1188.0f / 44100.0f,
                          1277.0f / 44100.0f, 1356.0f / 44100.0f,
                          1422.0f / 44100.0f, 1491.0f / 44100.0f,
                          1557.0f / 44100.0f, 1617.0f / 44100.0f};

  float allpassTimes[4] = {225.0f / 44100.0f, 556.0f / 44100.0f,
                            441.0f / 44100.0f, 341.0f / 44100.0f};

  CombFilterFeedback *fbcf[8];
  AllPassFilter *apf[4];

 public:
  void setDelayTimes(float factor);
  void setFeedback(float val);
  Freeverb(float fs = 41000.0f, float delayFactor = 5.0f,
           float feedback = 0.84f);
  float process(float in);
};