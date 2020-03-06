/////////////////////////////////////////////////////////////////////////////////
// loopback.cpp: 
// Connects input audio directly into output
//
// Set Make Parameters in Bela IDE: LDLIBS= -lcsound
//
// (c) V Lazzarini, 2018

#include <Bela.h>
#include <libraries/csound/csound.h>
#include <string>

struct CsData {
  Csound *csound;
  int bframes;
  int res;
  int frames;
};
  
static CsData gCsData;

bool setup(BelaContext *context, void *Data)
{
  Csound *csound;
  int res;
  // simple sinewave instrument
  std::string code = R"orc(
  nchnls = 2;
  ksmps = 32;
  instr 1
   a1, a2 ins
   outs a1, a2
  endin
  schedule(1, 0, -1);
  )orc"; 

  // Create a Csound engine
  csound = new Csound();
  csound->SetHostImplementedAudioIO(1,0);
  csound->SetOption("-odac");
  csound->SetOption("-iadc");
  csound->SetOption("--realtime");
  csound->SetOption("--daemon");

  // Compile orc code
  res = csound->CompileOrc(code.c_str());
  if(res == 0) {
    gCsData.res = res;
    gCsData.bframes = csound->GetKsmps();
    gCsData.frames = 0;
    gCsData.csound = csound;
    csound->Start();
    return true;
  } else return false;
}

void render(BelaContext *context, void *Data)
{
  if(gCsData.res == 0) {
    int n, i;
    int res = gCsData.res;
    int frames = gCsData.frames;
    int bframes = gCsData.bframes;
    Csound *csound = gCsData.csound;
    MYFLT scal = csound->Get0dBFS();
    MYFLT* audioIn = csound->GetSpin();
    MYFLT* audioOut = csound->GetSpout();
    int nchnls = csound->GetNchnls();

    // set the number of channels to
    // write to output
    nchnls = nchnls < context->audioOutChannels ?
      nchnls : context->audioOutChannels;
    
    // process 
    for(n = 0; n < context->audioFrames; n++, frames++){

      // if we run out of frames to output
      // call Csound to process another block
      if(frames == bframes) {
	if((res = csound->PerformKsmps()) == 0)
          frames = 0;
	else break;
      }
      
      // read/write audio data
      for(i = 0; i < nchnls; i++) {
        audioIn[frames*nchnls+i] = audioRead(context, n, i)*scal;
	audioWrite(context, n, i, audioOut[frames*nchnls+i]/scal);
      }
      
    }
    gCsData.res = res;
    gCsData.frames = frames;
  }
}

void cleanup(BelaContext *context, void *Data)
{
  delete gCsData.csound;
}


