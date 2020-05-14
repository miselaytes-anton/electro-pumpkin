#include <Bela.h>
#include <cmath>
#include <vector>

#include "I2C_MPR121/I2C_MPR121.h"
#include "dsp/ADSR.h"
#include "dsp/Biquad.h"
#include "dsp/CombFilterFeedback.h"
#include "dsp/Drum.h"
#include "dsp/Oscillator.h"
#include "dsp/OscillatorHarmonics.h"

#define NUM_TOUCH_PINS 12

using namespace std;
int gAudioFramesPerAnalogFrame = 0;

CombFilterFeedback *combFilterFeedback;
vector<OscillatorHarmonics> oscillators;
vector<Drum> drums;
vector<ADSR> envelopes;
Biquad lowPassFilter;
Oscillator lfo;

/**
 * Audio parameters
 **/
float biquadQFactor = 0.707;
float biquadPeakGain = 1;
float delayLength = 2;
float delayDecay = 0.97f;
float audioInputGain = 0.6f;
float gAttack = 0.1;  // Envelope attack (seconds)
float gDecay = 0.25;  // Envelope decay (seconds)
float gRelease = 3.0; // Envelope release (seconds)
float gSustain = 1.0; // Envelope sustain level
float lowPassFilterFc = 0;
float lfoFreq = 0.5;
float lfoDepth = 50;
float lowPassRangeBottom = 200;
float volume = 0.7;
// 12 notes of a C major scale
const vector<float> gFrequencies = {261.63, 293.66, 329.63, 349.23,
                                    392.00, 440.00, 493.88, 523.25,
                                    587.33, 659.25, 698.25, 783.99};

/**
 * MPR 121 parameters
 **/

I2C_MPR121 mpr121;     // Object to handle MPR121 sensing
AuxiliaryTask i2cTask; // Auxiliary task to read I2C

const vector<int> activePins = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
int readInterval = 50; // how often the MPR121 is read (in Hz)
float sensorValue[NUM_TOUCH_PINS];
int readCount = 0;           // How long until we read again...
int readIntervalSamples = 0; // How many samples between reads

void readMPR121(void *) {
  float normalizeFactor = 400.f; // ensure we get values from 0 to 1
  int touchThreshold = 3;        //  minimum amount of touch for trigger
  for (unsigned i : activePins) {
    sensorValue[i] = mpr121.getSensorValue(i, touchThreshold) / normalizeFactor;
  }
}

void setEnvelopeGate(ADSR *envelope, float amplitude) {
  unsigned envState = envelope->getState();
  if (amplitude > 0 &&
      (envState == envState::env_idle || envState == envState::env_release)) {
    envelope->gate(true);
  } else if (amplitude == 0 && envState != envState::env_idle) {
    envelope->gate(false);
  }
}

void setDelayParams(BelaContext *context, int analogFrame) {
  int delayLengthChanel = 0;
  int delayDecayChanel = 1;
  float sensitivity = 0.2;

  float newDelayLength =
      map(analogRead(context, analogFrame, delayLengthChanel), 0, 1, 0.01, 4.2);

  if (abs(newDelayLength - delayLength) >= sensitivity) {
    delayLength = newDelayLength;
    combFilterFeedback->setDelayLength(newDelayLength);
  }

  delayDecay =
      map(analogRead(context, analogFrame, delayDecayChanel), 0, 1, 0.1, 0.98);

  combFilterFeedback->setFeedback(delayDecay);
}

float getLowPassFilterFc() {
  return (lowPassRangeBottom + lfo.process() * lfoDepth);
}

bool setup(BelaContext *context, void *userData) {
  if (!mpr121.begin()) {
    rt_printf("Error initialising MPR121\n");
    return false;
  }

  i2cTask = Bela_createAuxiliaryTask(readMPR121, 50, "bela-mpr121");
  readIntervalSamples = context->audioSampleRate / readInterval;

  if (context->analogFrames) {
    gAudioFramesPerAnalogFrame = context->audioFrames / context->analogFrames;
  }

  for (unsigned i : activePins) {
    // Melody
    OscillatorHarmonics oscillator{
        gFrequencies[i] / 2, context->audioSampleRate, Oscillator::sawtooth, 2,
        OscillatorHarmonics::uneven};
    oscillators.push_back(oscillator);

    // Envelopes
    ADSR envelope;
    envelope.setAttackRate(gAttack * context->audioSampleRate);
    envelope.setDecayRate(gDecay * context->audioSampleRate);
    envelope.setReleaseRate(gRelease * context->audioSampleRate);
    envelope.setSustainLevel(gSustain);
    envelopes.push_back(envelope);

    // Drums
    Drum drum{context->audioSampleRate, gFrequencies[i] / 2};
    drums.push_back(drum);
  }

  // Effects
  lfo.setup(context->audioSampleRate, lfoFreq);
  lowPassFilter.setBiquad(bq_type_lowpass,
                          lowPassFilterFc / context->audioSampleRate,
                          biquadQFactor, biquadPeakGain);
  combFilterFeedback = new CombFilterFeedback(context->audioSampleRate,
                                              delayLength, 5, delayDecay);

  return true;
}

void render(BelaContext *context, void *userData) {
  for (unsigned n = 0; n < context->audioFrames; n++) {
    // schedule touch sensor readings
    if (++readCount >= readIntervalSamples) {
      readCount = 0;
      Bela_scheduleAuxiliaryTask(i2cTask);
    }

    // update parameter values
    lowPassFilterFc = getLowPassFilterFc();
    lowPassFilter.setFc(lowPassFilterFc / context->audioSampleRate);
    if (!(n % gAudioFramesPerAnalogFrame)) {
      setDelayParams(context, n / gAudioFramesPerAnalogFrame);
      volume = analogRead(context, n / gAudioFramesPerAnalogFrame, 2);
    }

    // drum processing
    float drumSample = 0.0;
    float micInput = audioRead(context, n, 0);
    for (unsigned i : activePins) {
      if (sensorValue[i] < 0.01) {
        continue;
      }
      drumSample += drums[i].process(micInput);
    }

    // melody processing
    float melodySample = 0.0;
    for (unsigned i : activePins) {
      setEnvelopeGate(&envelopes[i], sensorValue[i]);
      if (sensorValue[i] < 0.01 &&
          envelopes[i].getState() == envState::env_idle) {
        continue;
      }
      melodySample += envelopes[i].process() * oscillators[i].process();
    }
    melodySample = lowPassFilter.process(melodySample);

    // mixing and out
    float mix = melodySample / 6 + drumSample;
    mix = volume * combFilterFeedback->process(mix);
    for (unsigned ch = 0; ch < context->audioInChannels; ch++) {
      audioWrite(context, n, ch, mix);
    }
  }
}

void cleanup(BelaContext *context, void *userData) {}
