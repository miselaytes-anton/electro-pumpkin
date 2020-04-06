#include <Bela.h>
#include <stk/ADSR.h>
#include <stk/BlitSaw.h>
#include <stk/Generator.h>
#include <stk/JCRev.h>
#include <stk/OnePole.h>
#include <stk/SineWave.h>

#include <cmath>

#include "I2C_MPR121/I2C_MPR121.h"

#define NUM_TOUCH_PINS 12  // How many pins there are

using namespace stk;

int gAudioFramesPerAnalogFrame = 0;

StkFrames frames;
StkFrames envelopeFrames;
StkFrames tmpFrames;

BlitSaw *oscillators[NUM_TOUCH_PINS];
ADSR *envelopes[NUM_TOUCH_PINS];  // ADSR envelope

auto lowPassFilter = new OnePole{};
auto lfo = new SineWave{};
auto reverb = new JCRev{};

/**
 * Audio params
 **/
StkFloat amplitudeThreshold = 0.01;
StkFloat gAttack = 0.1;   // Envelope attack (seconds)
StkFloat gDecay = 0.25;   // Envelope decay (seconds)
StkFloat gRelease = 0.5;  // Envelope release (seconds)
StkFloat gSustain = 1.0;  // Envelope sustain level
StkFloat lowPassFilterFc = 400;
StkFloat lfoFreq = 0.5;
StkFloat lfoDepth = 50;
StkFloat lowPassRangeBottom = 200;
StkFloat lowPassRangeTop = 2500;
StkFloat volume = 0.7;
// 12 notes of a C major scale...
StkFloat frequencies[NUM_TOUCH_PINS] = {261.63, 293.66, 329.63, 349.23,
                                        392.00, 440.00, 493.88, 523.25,
                                        587.33, 659.25, 698.25, 783.99};
int numHarmonics = 1;

/**
 * MPR 121 params
 **/
float sensorValue[NUM_TOUCH_PINS];
I2C_MPR121 mpr121;                // Object to handle MPR121 sensing
AuxiliaryTask i2cTask;  // Auxiliary task to read I2C
int threshold = 0;  // Change this threshold to set the minimum amount of touch
float normalizeValue = 400.f;

void readMPR121(void *) {
  for (int i = 0; i < NUM_TOUCH_PINS; i++) {
    sensorValue[i] = mpr121.getSensorValue(i, threshold) / normalizeValue;
  }
}

void setEnvelopeGate(ADSR *envelope, float amplitude) {
  unsigned int envState = envelope->getState();
  if (amplitude > amplitudeThreshold &&
      (envState == ADSR::IDLE || envState == ADSR::RELEASE)) {
    envelope->keyOn();
  } else if (amplitude < amplitudeThreshold && envState != ADSR::IDLE) {
    envelope->keyOff();
  }
}

void setLowPass(StkFloat normalizedFrequency, OnePole *lowPassFilter) {
  // x(n) * a0 + y(n-1) * b1;
  StkFloat x = expf(-2.0f * (StkFloat)M_PI * normalizedFrequency);
  StkFloat a0 = 1.0f - x;
  StkFloat b1 = x;

  lowPassFilter->setCoefficients(a0, -b1);
}

StkFloat getLowPassFilterFc(BelaContext *context) {
  StkFloat value =
      map(analogRead(context, 0, 0), 0, 1, lowPassRangeBottom, lowPassRangeTop);
  return (value + lfo->tick() * lfoDepth);
}

void audioWriteFrames(BelaContext *context, StkFrames &frames) {
  for (unsigned int frame = 0; frame < context->audioFrames; frame++) {
    for (unsigned int channel = 0; channel < context->audioOutChannels;
         channel++) {
      float out = frames(frame, channel) * 0.1;
      audioWrite(context, frame, channel, out);
    }
  }
}

void resetFrames() {
  for (int i = 0; i < frames.size(); i++) {
    frames[i] = 0;
    tmpFrames[i] = 0;
    envelopeFrames[i] = 0;
  }
}

bool setup(BelaContext *context, void *userData) {
  if (!mpr121.begin(1, 0x5B)) {
    rt_printf("Error initialising MPR121\n");
    return false;
  }

  i2cTask = Bela_createAuxiliaryTask(readMPR121, 50, "bela-mpr121");

  Stk::setSampleRate(context->audioSampleRate);
  frames.resize(context->audioFrames, context->audioOutChannels);
  tmpFrames.resize(context->audioFrames, context->audioOutChannels);
  envelopeFrames.resize(context->audioFrames, context->audioOutChannels);

  for (unsigned int i = 0; i < NUM_TOUCH_PINS; i++) {
    oscillators[i] = new BlitSaw{};
    oscillators[i]->setFrequency(frequencies[i] / 2);
    oscillators[i]->setHarmonics(numHarmonics);

    // Set ADSR parameters
    envelopes[i] = new ADSR{};
    envelopes[i]->setAttackTime(gAttack);
    envelopes[i]->setDecayTime(gDecay);
    envelopes[i]->setReleaseTime(gRelease);
    envelopes[i]->setSustainLevel(gSustain);
  }

  lfo->setFrequency(lfoFreq * context->audioFrames);
  setLowPass(lowPassFilterFc, lowPassFilter);

  if (context->analogFrames) {
    gAudioFramesPerAnalogFrame = context->audioFrames / context->analogFrames;
  }

  return true;
}

void render(BelaContext *context, void *userData) {
  Bela_scheduleAuxiliaryTask(i2cTask);
  resetFrames();

  for (int i = 0; i < NUM_TOUCH_PINS; i++) {
    float amplitude = sensorValue[i];
    if (i >= 4 && i <= 7) {
      // sensor 4-7 are not connected, and we do not want to skip
      // notes
      continue;
    }
    if (amplitude < amplitudeThreshold &&
        envelopes[i]->getState() == ADSR::IDLE) {
      continue;
    }

    setEnvelopeGate(envelopes[i], amplitude);

    oscillators[i]->tick(tmpFrames);
    envelopes[i]->tick(envelopeFrames);
    tmpFrames *= envelopeFrames;
    frames += tmpFrames;
  }
  setLowPass(getLowPassFilterFc(context) / context->audioSampleRate,
             lowPassFilter);
  lowPassFilter->tick(frames);
  reverb->tick(frames);

  audioWriteFrames(context, frames);
}

void cleanup(BelaContext *context, void *userData) {}
