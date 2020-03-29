/***** allpass_filter *****/
#include <stdio.h>
#include <stdlib.h>

#include "allpass_filter.h"

// based on CCRMA:
// https://ccrma.stanford.edu/realsimple/Delay/Allpass_Filters_I.html

allpass_filter::allpass_filter(float sample_rate, float max_delay_seconds) {
  _read_pos = 0;
  //_write_pos=0;
  prev_val = 0;

  _sample_rate = sample_rate;
  _delay_line_max = max_delay_seconds * sample_rate;
  _delay_line = new float[_delay_line_max];

  set_delay_time(max_delay_seconds * 0.5f);
  set_feedback(0.75f);

  for (int i = 0; i < _delay_line_max; i++) _delay_line[i] = 0.0f;

}

void allpass_filter::set_delay_time(float dt) {
  int new_delay = dt * _sample_rate;
  if (new_delay == _current_delay) return;

  _current_delay = new_delay;

  if (_current_delay < 1) _current_delay = 1;
  if (_current_delay >= _delay_line_max) _current_delay = _delay_line_max - 1;
}

void allpass_filter::set_delay_samples(int delay_samples) {
  _current_delay = delay_samples;

  if (_current_delay < 1) _current_delay = 1;
  if (_current_delay >= _delay_line_max) _current_delay = _delay_line_max - 1;
}

void allpass_filter::set_feedback(
    float f)  // feedback corresponds to the Am parameter in the CCRMA writeup
{
  _feedback = f;
}

float allpass_filter::tick(float input) {
  float read_val =
      _delay_line[_read_pos];  // read the oldest value from the delay line

  float delay_input =
      input -
      _feedback *
          read_val;  // used to use + operation, but in CCRMA says should be -

  _delay_line[_read_pos] =
      delay_input;  // store the low passed value in the delay line

  _read_pos = (_read_pos + 1) % _current_delay;

  return delay_input * _feedback + read_val;
}