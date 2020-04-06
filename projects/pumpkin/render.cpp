#include <Bela.h>
#include <libraries/ADSR/ADSR.h>
#include <libraries/Oscillator/Oscillator.h>
#include <stk/JCRev.h>

#include <cmath>

#include "Biquad/Biquad.h"
#include "I2C_MPR121/I2C_MPR121.h"
#include "OscillatorHarmonics/OscillatorHarmonics.h"

#define NUM_TOUCH_PINS 12

using namespace stk;

int gAudioFramesPerAnalogFrame = 0;

OscillatorHarmonics oscillators[NUM_TOUCH_PINS];
ADSR envelopes[NUM_TOUCH_PINS];
Biquad lowPassFilter = Biquad();
Oscillator lfo;
JCRev reverb = JCRev();

/**
 * Audio parameters
 **/
float gAttack = 0.1;   // Envelope attack (seconds)
float gDecay = 0.25;   // Envelope decay (seconds)
float gRelease = 3.0;  // Envelope release (seconds)
float gSustain = 1.0;  // Envelope sustain level
float lowPassFilterFc = 0;
float lfoFreq = 0.5;
float lfoDepth = 50;
float lowPassRangeBottom = 200;
float lowPassRangeTop = 2500;
float volume = 0.7;
// 12 notes of a C major scale
float gFrequencies[NUM_TOUCH_PINS] = {261.63, 293.66, 329.63, 349.23,
                                      392.00, 440.00, 493.88, 523.25,
                                      587.33, 659.25, 698.25, 783.99};

/**
 * MPR 121
 **/
int readInterval = 50;  // how often the MPR121 is read (in Hz)
float sensorValue[NUM_TOUCH_PINS];
int readCount = 0;            // How long until we read again...
int readIntervalSamples = 0;  // How many samples between reads

I2C_MPR121 mpr121;      // Object to handle MPR121 sensing
AuxiliaryTask i2cTask;  // Auxiliary task to read I2C

void readMPR121(void *) {
  float normalizeFactor = 400.f;  // ensure we get values from 0 to 1
  int touchThreshold = 5;      //  minimum amount of touch for trigger
  for (int i = 0; i < NUM_TOUCH_PINS; i++) {
    sensorValue[i] = mpr121.getSensorValue(i, touchThreshold) / normalizeFactor;
  }
}

void setEnvelopeGate(ADSR *envelope, float amplitude) {
  unsigned int envState = envelope->getState();
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
  if (!mpr121.begin(1, 0x5B)) {
    rt_printf("Error initialising MPR121\n");
    return false;
  }

  i2cTask = Bela_createAuxiliaryTask(readMPR121, 50, "bela-mpr121");
  readIntervalSamples = context->audioSampleRate / readInterval;

  for (unsigned int i = 0; i < NUM_TOUCH_PINS; i++) {
    oscillators[i] =
        OscillatorHarmonics{gFrequencies[i] / 2, context->audioSampleRate,
                            Oscillator::sawtooth, 3, OscillatorHarmonics::even};

    // Set ADSR parameters
    envelopes[i].setAttackRate(gAttack * context->audioSampleRate);
    envelopes[i].setDecayRate(gDecay * context->audioSampleRate);
    envelopes[i].setReleaseRate(gRelease * context->audioSampleRate);
    envelopes[i].setSustainLevel(gSustain);
  }
  lfo.setup(lfoFreq, context->audioSampleRate);
  lowPassFilter.setBiquad(bq_type_lowpass,
                          lowPassFilterFc / context->audioSampleRate, 0.707, 1);
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
    lowPassFilterFc =
        getLowPassFilterFc(context, n / gAudioFramesPerAnalogFrame);
    lowPassFilter.setFc(lowPassFilterFc / context->audioSampleRate);

    float sample = 0.0;

    for (int i = 0; i < NUM_TOUCH_PINS; i++) {
      if (i >= 4 && i <= 7) {
        // sensor 4-7 are not connected, and we do not want to skip
        // notes
        continue;
      }

      setEnvelopeGate(&envelopes[i], sensorValue[i]);
      sample += envelopes[i].process() * oscillators[i].process();
    }
    sample /= NUM_TOUCH_PINS;

    float out = reverb.tick(lowPassFilter.process(sample)) * volume;
    for (unsigned int ch = 0; ch < context->audioInChannels; ch++) {
      audioWrite(context, n, ch, out);
    }
  }
}

void cleanup(BelaContext *context, void *userData) {}
