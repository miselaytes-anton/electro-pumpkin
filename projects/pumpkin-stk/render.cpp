#include <Bela.h>
#include <stk/ADSR.h>
#include <stk/BlitSaw.h>
#include <stk/FreeVerb.h>
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
BlitSaw *oscillators[NUM_TOUCH_PINS];

StkFloat gAttack = 0.1;           // Envelope attack (seconds)
StkFloat gDecay = 0.25;           // Envelope decay (seconds)
StkFloat gRelease = 2;          // Envelope release (seconds)
StkFloat gSustain = 1.0;          // Envelope sustain level
ADSR *envelopes[NUM_TOUCH_PINS];  // ADSR envelope

OnePole *lpFilter;
StkFloat lpFilterFc = 0;

SineWave *lfo;
StkFloat lfoFreq = 0.5;
int lfoDepth = 150;

FreeVerb *freeverb;
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
  if (amplitude > 0.01 &&
      (envState == ADSR::IDLE || envState == ADSR::RELEASE)) {
    envelope->keyOn();
  } else if (amplitude < 0.01 && envState != ADSR::IDLE) {
    envelope->keyOff();
  }
}

void setLowPass(StkFloat frequency, OnePole *lfo) {
  // input * a0 + ym1 * b1;
  StkFloat a1 = expf(-2.0f * (StkFloat)M_PI * frequency);
  StkFloat b0 = 1.0f - a1;

  // b_[0] * inputs_[0] - a_[1] * outputs_[1];
  lfo->setCoefficients(a1, b0);
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

  lfo = new SineWave{};
  lfo->setFrequency(lfoFreq / context->audioSampleRate);
  // setLowPass(lpFilterFc, lpFilter);

  freeverb = new FreeVerb{};
  freeverb->setMode(1);
  freeverb->setEffectMix(0.1);
  freeverb->setRoomSize(0.1);
  freeverb->setDamping(0.1);
  freeverb->setWidth(0.1);

  if (context->analogFrames) {
    gAudioFramesPerAnalogFrame = context->audioFrames / context->analogFrames;
  }

  return true;
}

void render(BelaContext *context, void *userData) {
  for (unsigned int n = 0; n < context->audioFrames; n++) {
    // schedule touch sensor readings
    if (++readCount >= readIntervalSamples) {
      readCount = 0;
      Bela_scheduleAuxiliaryTask(i2cTask);
    }
    lpFilterFc = map(analogRead(context, n / gAudioFramesPerAnalogFrame, 0), 0,
                     0.8, 200, 2500);
    // setLowPass((lpFilterFc + lfo->tick() *
    // lfoDepth)/context->audioSampleRate, lpFilter);

    StkFloat sample = 0.0;

    int numPressed = 1;
    for (int i = 0; i < NUM_TOUCH_PINS; i++) {
      if (i >= 4 && i <= 7) {
        // sensor 4-7 are not connected, and we do not want to skip
        // notes
        continue;
      }

      float amplitude = sensorValue[i] / 400.f;

      setEnvelopeGate(envelopes[i], amplitude);
      if (envelopes[i]->getState() != ADSR::IDLE) {
        numPressed++;
      }
      sample += envelopes[i]->tick() * oscillators[i]->tick();
    }
    StkFloat out = freeverb->tick(sample/ numPressed) ;
    for (unsigned int ch = 0; ch < context->audioInChannels; ch++) {
      context->audioOut[context->audioInChannels * n + ch] = out * volume;
    }
  }
}

void cleanup(BelaContext *context, void *userData) {}

// Auxiliary task to read the I2C board
void readMPR121(void *) {
  for (int i = 0; i < NUM_TOUCH_PINS; i++) {
    sensorValue[i] = mpr121.getSensorValue(i, threshold);
  }
}
