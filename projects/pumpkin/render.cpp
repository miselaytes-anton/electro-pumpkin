#include <Bela.h>
#include <libraries/ADSR/ADSR.h>
#include <libraries/Oscillator/Oscillator.h>
#include <cmath>

#include "Biquad/Biquad.h"
#include "I2C_MPR121/I2C_MPR121.h"
#include "OscillatorHarmonics/OscillatorHarmonics.h"
#include "Freeverb/Freeverb.h"

#define NUM_TOUCH_PINS 12  // How many pins there are

int gAudioFramesPerAnalogFrame = 0;

// 12 notes of a C major scale...
float gFrequencies[NUM_TOUCH_PINS] = {261.63, 293.66, 329.63, 349.23,
                                      392.00, 440.00, 493.88, 523.25,
                                      587.33, 659.25, 698.25, 783.99};
OscillatorHarmonics oscillators[NUM_TOUCH_PINS];

float gAttack = 0.1;             // Envelope attack (seconds)
float gDecay = 0.25;             // Envelope decay (seconds)
float gRelease = 3.0;            // Envelope release (seconds)
float gSustain = 1.0;            // Envelope sustain level
ADSR envelopes[NUM_TOUCH_PINS];  // ADSR envelope
Biquad lpFilter = Biquad();
float lpFilterFc = 0;
Oscillator lfo;
float lfoFreq = 0.5;
int lfoDepth = 150;
Freeverb *freeverb;
float volume = 0.5;

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
      (envState == envState::env_idle || envState == envState::env_release)) {
    envelope->gate(true);
  } else if (amplitude < 0.01 && envState != envState::env_idle) {
    envelope->gate(false);
  }
}

bool setup(BelaContext *context, void *userData) {
  if (!mpr121.begin(1, 0x5B)) {
    rt_printf("Error initialising MPR121\n");
    return false;
  }

  i2cTask = Bela_createAuxiliaryTask(readMPR121, 50, "bela-mpr121");
  readIntervalSamples = context->audioSampleRate / readInterval;

  for (unsigned int i = 0; i < NUM_TOUCH_PINS; i++) {
    oscillators[i] = OscillatorHarmonics{
        gFrequencies[i]/2, context->audioSampleRate, Oscillator::triangle, 3,
        OscillatorHarmonics::uneven};

    // Set ADSR parameters
    envelopes[i].setAttackRate(gAttack * context->audioSampleRate);
    envelopes[i].setDecayRate(gDecay * context->audioSampleRate);
    envelopes[i].setReleaseRate(gRelease * context->audioSampleRate);
    envelopes[i].setSustainLevel(gSustain);
  }
  lfo.setup(lfoFreq, context->audioSampleRate);
  lpFilter.setBiquad(bq_type_lowpass, lpFilterFc / context->audioSampleRate,
                     0.707, 1);
  freeverb = new Freeverb(context->audioSampleRate);
  freeverb->set_delay_times(1.0);
  freeverb->set_feedback(0.5);
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
    lpFilter.setFc((lpFilterFc + lfo.process() * lfoDepth) /
                   context->audioSampleRate);

    float sample = 0.0;

    int numPressed = 1;
    for (int i = 0; i < NUM_TOUCH_PINS; i++) {
      if (i >= 4 && i <= 7) {
        // sensor 4-7 are not connected, and we do not want to skip
        // notes
        continue;
      }

      float amplitude = sensorValue[i] / 400.f;
      setEnvelopeGate(&envelopes[i], amplitude);
      if (envelopes[i].getState() != envState::env_idle) {
        numPressed++;
      }
      sample += envelopes[i].process() * oscillators[i].process();
    }
    // todo: improve/remove filter as adds noise
    float out = lpFilter.process(sample / numPressed);
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
