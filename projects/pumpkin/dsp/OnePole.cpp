#include "OnePole.h"

#include <math.h>
#include <stdio.h>

OnePole::OnePole() {}

OnePole::OnePole(float fs, float fc, int type) { 
   ym1 = 0.0;  // Reset filter state
  _fs = fs;
  setFilter(_fs, fc, type);
}

OnePole::OnePole(float fs, float a0, float b1) { 
   ym1 = 0.0;  // Reset filter state
  _fs = fs;
  a0 = a0;
  b1 = b1;
}


void OnePole::setFilter(float fs, float fc,  int type) {
  _fs = fs;
  setType(type);
  setFc(fc);
}

void OnePole::setFc(float fc) {
  if (_type == LP) {
    b1 = expf(-2.0f * (float)M_PI * fc / _fs);
    a0 = 1.0f - b1;
  } else if (_type == HP) {
    b1 = -expf(-2.0f * (float)M_PI * (0.5f - fc / _fs));
    a0 = 1.0f + b1;
  }
  _fc = fc;
}

void OnePole::setType(int type) {
  if (type == LP || type == HP) {
    _type = type;
  } else {
    fprintf(stderr, "Invalid type\n");
  }
}

float OnePole::process(float input) { return ym1 = input * a0 + ym1 * b1; }

OnePole::~OnePole() { cleanup(); }

void OnePole::cleanup() {}
