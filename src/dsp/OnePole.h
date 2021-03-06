/*
 * Adapted from: http://www.earlevel.com/main/2012/12/15/a-one-pole-filter/
 *
 *	by: Adán L. Benito
 *	on: November 2018
 *	original code by Nigel Redmon
 *
 */

class OnePole {
private:
  float _fc;
  float _fs;
  int _type;

  float a0, b1, ym1;

  void setType(int type);
  void setFc(float fc);

public:
  OnePole();
  OnePole(float fs, float fc, int type = LP);
  OnePole(float fs, float a0, float b1);

  ~OnePole();

  void cleanup();

  enum Type { LP = 0, HP = 1 };

  void setFilter(float fs, float fc, int type);

  inline float process(float input){
    return ym1 = input * a0 + ym1 * b1;
  };
};
