#include <vector>

using namespace std;

class Delay {
 private:
  vector<float> _samples;
  int _readPosition;
  int _writePosition;
  float _delayLength;
  float _maxDelayLength;
  float _feedback;

 public:
  Delay(float delayLength = 41000, float maxDelayLength = 41000,
        float feedback = 0);
  ~Delay();
  void setDelayLength(float delayLength);
  float process(float input);
};
