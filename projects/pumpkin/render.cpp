#include "dsp/ADSR.h"
#include "dsp/CombFilterFeedback.h"
#include "dsp/Oscillator.h"
#include <Bela.h>

#include <cmath>
#include <vector>

#include "I2C_MPR121/I2C_MPR121.h"
#include "_Freeverb/Freeverb.h"
#include "dsp/Biquad.h"
#include "dsp/OscillatorHarmonics.h"

#define NUM_TOUCH_PINS 12

using namespace std;

int gAudioFramesPerAnalogFrame = 0;
int sampleRate = 41000;

CombFilterFeedback *combFilterFeedback;
vector<OscillatorHarmonics> oscillators;
vector<ADSR> envelopes;
Biquad lowPassFilter;
Oscillator lfo;
Freeverb reverb;

I2C_MPR121 mpr121;     // Object to handle MPR121 sensing
AuxiliaryTask i2cTask; // Auxiliary task to read I2C

/**
 * Audio parameters
 **/
float reverbDelayLength = 1;
float reverbFeedback = 0.2;
float biquadQFactor = 0.707;
float biquadPeakGain = 1;
float delayLength = 2;
float delayDecay = 0.75f;
float audioInputGain = 0.2f;
float gAttack = 0.1;  // Envelope attack (seconds)
float gDecay = 0.25;  // Envelope decay (seconds)
float gRelease = 3.0; // Envelope release (seconds)
float gSustain = 1.0; // Envelope sustain level
float lowPassFilterFc = 0;
float lfoFreq = 0.5;
float lfoDepth = 50;
float lowPassRangeBottom = 200;
float lowPassRangeTop = 2500;
float volume = 0.7;
// 12 notes of a C major scale
const vector<float> gFrequencies = {261.63, 293.66, 329.63, 349.23,
                                    392.00, 440.00, 493.88, 523.25,
                                    587.33, 659.25, 698.25, 783.99};

/**
 * MPR 121 parameters
 **/
const vector<int> activePins = {0, 1, 2, 3, 5, 6, 7, 8, 9, 10, 11};
int readInterval = 50; // how often the MPR121 is read (in Hz)
float sensorValue[NUM_TOUCH_PINS];
int readCount = 0;           // How long until we read again...
int readIntervalSamples = 0; // How many samples between reads

void readMPR121(void *) {
  float normalizeFactor = 400.f; // ensure we get values from 0 to 1
  int touchThreshold = 5;        //  minimum amount of touch for trigger
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

float getLowPassFilterFc(BelaContext *context, int analogFrameIndex) {
  float value = map(analogRead(context, analogFrameIndex, 0), 0, 1,
                    lowPassRangeBottom, lowPassRangeTop);
  return (value + lfo.process() * lfoDepth);
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
        gFrequencies[i] / 2, context->audioSampleRate, Oscillator::sawtooth, 1,
        OscillatorHarmonics::even};
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

  reverb.set_delay_times(reverbDelayLength);
  reverb.set_feedback(reverbFeedback);
  combFilterFeedback = new CombFilterFeedback(
      context->audioSampleRate, delayLength, delayLength, delayDecay);

  if (context->analogFrames) {
    gAudioFramesPerAnalogFrame = context->audioFrames / context->analogFrames;
  }

  return true;
}

void render(BelaContext *context, void *userData) {
  for (unsigned n = 0; n < context->audioFrames; n++) {
    // schedule touch sensor readings
    if (++readCount >= readIntervalSamples) {
      readCount = 0;
      Bela_scheduleAuxiliaryTask(i2cTask);
    }
    lowPassFilterFc =
        getLowPassFilterFc(context, n / gAudioFramesPerAnalogFrame);
    lowPassFilter.setFc(lowPassFilterFc / context->audioSampleRate);

    float sample = 0.0;

    for (unsigned i : activePins) {
      setEnvelopeGate(&envelopes[i], sensorValue[i]);
      sample += envelopes[i].process() * oscillators[i].process();
    }
    float mix = lowPassFilter.process(sample / 8) +
                audioInputGain * audioRead(context, n, 0);
    float out = volume * reverb.tick(combFilterFeedback->process(mix));
    for (unsigned ch = 0; ch < context->audioInChannels; ch++) {
      audioWrite(context, n, ch, out);
    }
  }
}

void cleanup(BelaContext *context, void *userData) {}
