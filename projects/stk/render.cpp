#include <Bela.h>
#include <stk/BlitSaw.h>
#include <stk/FreeVerb.h>


using namespace stk;
StkFrames frames;
BlitSaw sine;
FreeVerb freeverb;

bool setup(BelaContext *context, void *userData) {
  Stk::setSampleRate(context->audioSampleRate);
  frames.resize(context->audioFrames, context->audioOutChannels);
  sine.setFrequency(441.0);
  return true;
}

void render(BelaContext *context, void *userData) {
  for (unsigned int channel = 0; channel < context->audioOutChannels;
       channel++) {
    freeverb.tick(sine.tick(frames, channel), channel);
  }

  for (unsigned int frame = 0; frame < context->audioFrames; frame++) {
    for (unsigned int channel = 0; channel < context->audioOutChannels;
         channel++) {
	    float out = frames(frame, channel) * 0.1;
      audioWrite(context, frame, channel, out);
    }
  }
}

void cleanup(BelaContext *context, void *userData) {}
