/***** allpass_filter.h *****/
#ifndef DJZ_ALL_PASS_FILTER
#define DJZ_ALL_PASS_FILTER

class allpass_filter
{
public:
   allpass_filter(float sample_rate, float max_delay_seconds);

   void set_delay_time(float dt);
   void set_delay_samples(int delay_samples);
   void set_feedback(float f);
   
   float tick(float input);

private:
   unsigned int _delay_line_max;
   float *_delay_line;

   float _sample_rate;
   int   _current_delay;
   float _feedback;

   int _read_pos;
   //int _write_pos;
   
   float prev_val;
};


#endif