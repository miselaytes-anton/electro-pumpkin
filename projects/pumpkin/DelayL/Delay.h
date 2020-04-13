#include <vector>

using namespace std;

class Delay {
 private:
  vector<float> _samples;
  int long _writePosition;
  int long _delayLength;
  int long _maxDelayLength;
  float _feedback;

 public:
  Delay(float delayLength = 41000, int long maxDelayLength = 41000,
        float feedback = 0);
  ~Delay();
  void setDelayLength(float delayLength);
  float process(float input);
};
