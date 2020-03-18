#include <Bela.h>
#include <libraries/ADSR/ADSR.h>
#include <libraries/Oscillator/Oscillator.h>

#include <cmath>
#include "I2C_MPR121.h"
#include "OscillatorHarmonics.h"

// How many pins there are
#define NUM_TOUCH_PINS 12

// Prevent clipping
float volume = 0.5;

// Change this to change how often the MPR121 is read (in Hz)
int readInterval = 50;

// Change this threshold to set the minimum amount of touch
int threshold = 0;

// This array holds the continuous sensor values
int sensorValue[NUM_TOUCH_PINS];

// 12 notes of a C major scale...
float gFrequencies[NUM_TOUCH_PINS] = {261.63, 293.66, 329.63, 349.23,
                                      392.00, 440.00, 493.88, 523.25,
                                      587.33, 659.25, 698.25, 783.99};
OscillatorHarmonics oscillators[NUM_TOUCH_PINS];

float gAttack = 0.1;             // Envelope attack (seconds)
float gDecay = 0.25;             // Envelope decay (seconds)
float gRelease = 3.5;            // Envelope release (seconds)
float gSustain = 1.0;            // Envelope sustain level
ADSR envelopes[NUM_TOUCH_PINS];  // ADSR envelope

I2C_MPR121 mpr121;      // Object to handle MPR121 sensing
AuxiliaryTask i2cTask;  // Auxiliary task to read I2C

int readCount = 0;            // How long until we read again...
int readIntervalSamples = 0;  // How many samples between reads

void readMPR121(void *);

bool setup(BelaContext *context, void *userData) {
  if (!mpr121.begin(1, 0x5B)) {
    rt_printf("Error initialising MPR121\n");
    return false;
  }

  i2cTask = Bela_createAuxiliaryTask(readMPR121, 50, "bela-mpr121");
  readIntervalSamples = context->audioSampleRate / readInterval;

  for (unsigned int i = 0; i < NUM_TOUCH_PINS; i++) {
    oscillators[i] =
        OscillatorHarmonics{gFrequencies[i], context->audioSampleRate,
                            Oscillator::osc_type::triangle, 3};

    // Set ADSR parameters
    envelopes[i].setAttackRate(gAttack * context->audioSampleRate);
    envelopes[i].setDecayRate(gDecay * context->audioSampleRate);
    envelopes[i].setReleaseRate(gRelease * context->audioSampleRate);
    envelopes[i].setSustainLevel(gSustain);
  }

  return true;
}

void render(BelaContext *context, void *userData) {
  for (unsigned int n = 0; n < context->audioFrames; n++) {
    // Keep this code: it schedules the touch sensor readings
    if (++readCount >= readIntervalSamples) {
      readCount = 0;
      Bela_scheduleAuxiliaryTask(i2cTask);
    }

    float sample = 0.0;

    // This code can be replaced with your favourite audio code
    int numPressed = 1;
    for (int i = 0; i < NUM_TOUCH_PINS; i++) {
      // sensor 4-7 are not connected
      // and we do not want to skip notes
      if (i >= 4 && i <= 7) continue;
      float amplitude = sensorValue[i] / 400.f;
      unsigned int envState = envelopes[i].getState();
      if (amplitude > 0.01 && (envState == envState::env_idle ||
                               envState == envState::env_release)) {
        envelopes[i].gate(true);

      } else if (amplitude < 0.01 && envState != envState::env_idle) {
        envelopes[i].gate(false);
      }

      sample += envelopes[i].process() * oscillators[i].process();
      if (envState != envState::env_idle) {
        numPressed++;
      }
    }
    for (unsigned int ch = 0; ch < context->audioInChannels; ch++) {
      context->audioOut[context->audioInChannels * n + ch] =
          sample / numPressed;
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
