#pragma once

#include "AllPassFilter.h"
#include "CombFilterFeedback.h"
#include "OnePole.h"

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
  OnePole *lowpass[8];

public:
  void setDelayTimes(float factor);
  void setFeedback(float val);
  Freeverb(float fs = 41000.0f, float delayFactor = 5.0f,
           float feedback = 0.5f);
  inline float process(float in) {
    float out = 0.0f;

    // comb filters in parallel
    for (int i = 0; i < 8; i++) {
      out += fbcf[i]->process(in, [&](float delayedSample) -> float {
        return lowpass[i]->process(delayedSample) * 0.5;
      });
    }
    out /= 8;

    // sequence of all pass filters
    for (int i = 0; i < 2; i++) {
      out = apf[i]->process(out);
    }

    return out;
  };
};