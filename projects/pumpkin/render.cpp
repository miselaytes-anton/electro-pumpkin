#include "dsp/ADSR.h"
#include "dsp/CombFilterFeedback.h"
#include "dsp/Oscillator.h"
#include <Bela.h>

#include "I2C_MPR121/I2C_MPR121.h"
#include "dsp/Biquad.h"
#include "dsp/OscillatorHarmonics.h"
#include <cmath>
#include <vector>

#define NUM_TOUCH_PINS 12

using namespace std;
int gAudioFramesPerAnalogFrame = 0;

CombFilterFeedback *combFilterFeedback;
vector<OscillatorHarmonics> oscillators;
vector<ADSR> envelopes;
Biquad lowPassFilter;
Biquad delayLowPassFilter;
Oscillator lfo;

I2C_MPR121 mpr121;     // Object to handle MPR121 sensing
AuxiliaryTask i2cTask; // Auxiliary task to read I2C

/**
 * Audio parameters
 **/
float biquadQFactor = 0.707;
float biquadPeakGain = 1;
float delayLength = 2;
float delayLowPassFilterFc = 1700;
float delayDecay = 0.97f;
float audioInputGain = 0.4f;
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

  for (unsigned i : activePins) {
    OscillatorHarmonics oscillator{
        gFrequencies[i] / 2, context->audioSampleRate, Oscillator::sawtooth, 2,
        OscillatorHarmonics::uneven};
    oscillators.push_back(oscillator);

    // Set ADSR parameters
    ADSR envelope;
    envelope.setAttackRate(gAttack * context->audioSampleRate);
    envelope.setDecayRate(gDecay * context->audioSampleRate);
    envelope.setReleaseRate(gRelease * context->audioSampleRate);
    envelope.setSustainLevel(gSustain);
    envelopes.push_back(envelope);
  }

  lfo.setup(context->audioSampleRate, lfoFreq);
  lowPassFilter.setBiquad(bq_type_lowpass,
                          lowPassFilterFc / context->audioSampleRate,
                          biquadQFactor, biquadPeakGain);
  delayLowPassFilter.setBiquad(bq_type_lowpass,
                               delayLowPassFilterFc / context->audioSampleRate,
                               biquadQFactor, biquadPeakGain);

  combFilterFeedback = new CombFilterFeedback(
      context->audioSampleRate, delayLength, delayLength * 4, delayDecay);
  if (context->analogFrames) {
    gAudioFramesPerAnalogFrame = context->audioFrames / context->analogFrames;
  }

  return true;
}

void setDelayParams(BelaContext *context, int analogFrame) {
  int delayLengthChanel = 0;
  int delayDecayChanel = 1;
  float sensitivity = 0.2;
  float minimumDelayLength = 0.012;

  float newDelayLength =
      map(analogRead(context, analogFrame, delayLengthChanel), 0, 1, 0.01, 2.2);

  if (abs(newDelayLength - delayLength) >= sensitivity) {
    delayLength = newDelayLength;
    combFilterFeedback->setDelayLength(newDelayLength);
  } else if (newDelayLength < sensitivity && delayLength >= sensitivity) {
    delayLength = minimumDelayLength;
    combFilterFeedback->setDelayLength(minimumDelayLength);
  }

  delayDecay =
      map(analogRead(context, analogFrame, delayDecayChanel), 0, 1, 0.1, 0.98);

  combFilterFeedback->setFeedback(delayDecay);
}

void render(BelaContext *context, void *userData) {
  for (unsigned n = 0; n < context->audioFrames; n++) {
    if (++readCount >= readIntervalSamples) {
      // schedule touch sensor readings
      readCount = 0;
      Bela_scheduleAuxiliaryTask(i2cTask);
    }
    lowPassFilterFc = getLowPassFilterFc();
    lowPassFilter.setFc(lowPassFilterFc / context->audioSampleRate);
    if (!(n % gAudioFramesPerAnalogFrame)) {
      setDelayParams(context, n / gAudioFramesPerAnalogFrame);
      volume = analogRead(context, n / gAudioFramesPerAnalogFrame, 2);
    }

    float sample = 0.0;

    for (unsigned i : activePins) {
      setEnvelopeGate(&envelopes[i], sensorValue[i]);
      if (sensorValue[i] < 0.01 &&
          envelopes[i].getState() == envState::env_idle) {
        continue;
      }
      sample += envelopes[i].process() * oscillators[i].process();
    }
    sample /= activePins.size();
    float mix = lowPassFilter.process(sample) +
                audioInputGain * audioRead(context, n, 0);
    float out = volume * combFilterFeedback->process(
                             mix, [](float delayedSample) -> float {
                               return delayLowPassFilter.process(delayedSample);
                             });
    for (unsigned ch = 0; ch < context->audioInChannels; ch++) {
      audioWrite(context, n, ch, out);
    }
  }
}

void cleanup(BelaContext *context, void *userData) {}
