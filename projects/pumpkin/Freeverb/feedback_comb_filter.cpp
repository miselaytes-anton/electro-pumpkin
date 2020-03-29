/***** feedback_comb_filter *****/

//  based off CCRMA
//  https://ccrma.stanford.edu/~jos/pasp/Feedback_Comb_Filters.html

#include <stdio.h>
#include <stdlib.h>

#include "feedback_comb_filter.h"

feedback_comb_filter::feedback_comb_filter(float sample_rate,
                                           float max_delay_seconds) {
  _read_pos = 0;
  _write_pos = 0;
  prev_val = 0;

  _sample_rate = sample_rate;
  _delay_line_max = max_delay_seconds * sample_rate;
  _delay_line = new float[_delay_line_max];
  effectMix = 0.5f;

  set_delay_time(max_delay_seconds * 0.5f);
  set_feedback(0.75f);

  for (int i = 0; i < _delay_line_max; i++) _delay_line[i] = 0.0f;
}

void feedback_comb_filter::set_delay_time(float dt) {
  int new_delay = dt * _sample_rate;
  if (new_delay == _current_delay) return;

  _current_delay = new_delay;

  if (_current_delay < 1) _current_delay = 1;
  if (_current_delay >= _delay_line_max) _current_delay = _delay_line_max - 1;
}

void feedback_comb_filter::set_delay_samples(int delay_samples) {
  _current_delay = delay_samples;

  if (_current_delay < 1) _current_delay = 1;
  if (_current_delay >= _delay_line_max) _current_delay = _delay_line_max - 1;
}

void feedback_comb_filter::set_feedback(
    float f)  // feedback corresponds to the Am parameter in the CCRMA writeup
{
  _feedback = f;
}

float feedback_comb_filter::tick(float input) {
  float read_val =
      _delay_line[_read_pos];  // read the oldest value from the delay line

  float computed =
      input -
      _feedback *
          read_val;  // used to use + operation, but in CCRMA says should be -

  float delay_store_value = 0.2f * computed + prev_val * 0.8f;
  _delay_line[_write_pos] =
      delay_store_value;  // store the low passed value in the delay line
  prev_val = delay_store_value;

  _read_pos = (_read_pos + 1) % _current_delay;
  _write_pos = (_write_pos + 1) % _current_delay;

  return read_val;
}