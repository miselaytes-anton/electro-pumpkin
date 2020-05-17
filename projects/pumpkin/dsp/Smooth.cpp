// Exponential smoothing filter
// init with coeff, for me 0.05 is normal
// set initial value via setValue(some reading from sensor)
// in update loop call updateValue and folowing getValue for filtered

class Smooth {
private:
  float _value;
  float _coeff;

public:
  // coeff [0..1] if near to zero filter more inert
  Smooth(float coeff = 0.001) {
    _value = 0.0;
    _coeff = coeff;
  }

  // update inernal value of filter
  float process(float value) {
    _value = value * _coeff + _value * (1 - _coeff);
    return _value;
  }
};