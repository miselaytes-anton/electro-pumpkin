#include <Bela.h>
#include <stk/ADSR.h>
#include <stk/BlitSaw.h>
#include <stk/JCRev.h>
#include <stk/Generator.h>
#include <stk/OnePole.h>
#include <stk/SineWave.h>
#include <cmath>
#include "I2C_MPR121/I2C_MPR121.h"

using namespace stk;

#define NUM_TOUCH_PINS 12  // How many pins there are

int gAudioFramesPerAnalogFrame = 0;

// 12 notes of a C major scale...
StkFloat gFrequencies[NUM_TOUCH_PINS] = {261.63, 293.66, 329.63, 349.23,
                                         392.00, 440.00, 493.88, 523.25,
                                         587.33, 659.25, 698.25, 783.99};

StkFrames frames;
StkFrames envelopeFrames;
StkFrames tmpFrames;

BlitSaw *oscillators[NUM_TOUCH_PINS];
StkFloat amplitudeThreshold = 0.01;

StkFloat gAttack = 0.1;           // Envelope attack (seconds)
StkFloat gDecay = 0.25;           // Envelope decay (seconds)
StkFloat gRelease = 0.5;          // Envelope release (seconds)
StkFloat gSustain = 1.0;          // Envelope sustain level
ADSR *envelopes[NUM_TOUCH_PINS];  // ADSR envelope

auto lowPassFilter = new OnePole{};
StkFloat lowPassFilterFc = 400;

auto lfo = new SineWave{};
StkFloat lfoFreq = 2;
int lfoDepth = 300;

auto reverb = new JCRev {};
StkFloat volume = 0.5;

/* MPR 121 */
int readInterval =
    50;  // Change this to change how often the MPR121 is read (in Hz)
int threshold = 0;  // Change this threshold to set the minimum amount of touch
int sensorValue[NUM_TOUCH_PINS];  // This array holds the continuous sensor
                                  // values
I2C_MPR121 mpr121;                // Object to handle MPR121 sensing
AuxiliaryTask i2cTask;            // Auxiliary task to read I2C
int readCount = 0;                // How long until we read again...
int readIntervalSamples = 0;      // How many samples between reads
void readMPR121(void *);

void setEnvelopeGate(ADSR *envelope, float amplitude) {
  unsigned int envState = envelope->getState();
  if (amplitude > amplitudeThreshold &&
      (envState == ADSR::IDLE || envState == ADSR::RELEASE)) {
    envelope->keyOn();
  } else if (amplitude < amplitudeThreshold && envState != ADSR::IDLE) {
    envelope->keyOff();
  }
}

void setLowPass(StkFloat frequency, OnePole *lfo) {
  // input * a0 + ym1 * b1;
  StkFloat a1 = expf(-2.0f * (StkFloat)M_PI * frequency);
  StkFloat b0 = 1.0f - a1;

  // b_[0] * inputs_[0] - a_[1] * outputs_[1];
  lfo->setCoefficients(b0, a1);
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

bool setup(BelaContext *context, void *userData) {
  if (!mpr121.begin(1, 0x5B)) {
    rt_printf("Error initialising MPR121\n");
    return false;
  }

  i2cTask = Bela_createAuxiliaryTask(readMPR121, 50, "bela-mpr121");
  readIntervalSamples = context->audioSampleRate / readInterval;

  Stk::setSampleRate(context->audioSampleRate);
  frames.resize(context->audioFrames, context->audioOutChannels);
  tmpFrames.resize(context->audioFrames, context->audioOutChannels);
  envelopeFrames.resize(context->audioFrames, context->audioOutChannels);

  for (unsigned int i = 0; i < NUM_TOUCH_PINS; i++) {
    oscillators[i] = new BlitSaw{};
    oscillators[i]->setFrequency(gFrequencies[i] / 2);
    oscillators[i]->setHarmonics(3);

    // Set ADSR parameters
    envelopes[i] = new ADSR{};
    envelopes[i]->setAttackTime(gAttack);
    envelopes[i]->setDecayTime(gDecay);
    envelopes[i]->setReleaseTime(gRelease);
    envelopes[i]->setSustainLevel(gSustain);
  }

  lfo->setFrequency(lfoFreq / context->audioSampleRate);
  setLowPass(lowPassFilterFc, lowPassFilter);

  if (context->analogFrames) {
    gAudioFramesPerAnalogFrame = context->audioFrames / context->analogFrames;
  }

  return true;
}

void render(BelaContext *context, void *userData) {
  Bela_scheduleAuxiliaryTask(i2cTask);

  for (int i = 0; i < frames.size(); i++) {
    frames[i] = 0;
    tmpFrames[i] = 0;
    envelopeFrames[i] = 0;
  }

  for (int i = 0; i < NUM_TOUCH_PINS; i++) {
    float amplitude = sensorValue[i] / 400.f;
    if (i >= 4 && i <= 7) {
      // sensor 4-7 are not connected, and we do not want to skip
      // notes
      continue;
    }
    if (amplitude < amplitudeThreshold && envelopes[i]->getState() == ADSR::IDLE) {
      continue;
    }

    setEnvelopeGate(envelopes[i], amplitude);

    oscillators[i]->tick(tmpFrames);
    envelopes[i]->tick(envelopeFrames);
    tmpFrames *= envelopeFrames;
    frames += tmpFrames;
  }
  lowPassFilter->tick(frames);
  reverb->tick(frames);

  audioWriteFrames(context, frames);
}

void cleanup(BelaContext *context, void *userData) {}

// Auxiliary task to read the I2C board
void readMPR121(void *) {
  for (int i = 0; i < NUM_TOUCH_PINS; i++) {
    sensorValue[i] = mpr121.getSensorValue(i, threshold);
  }
}
