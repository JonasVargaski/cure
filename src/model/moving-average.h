#ifndef _MODEL_ANALOG_
#define _MODEL_ANALOG_

#include <stdint.h>

class MovingAverageModel
{
private:
  uint8_t _samples;
  uint16_t *_values;
  int _index;
  int32_t _sum;
  bool _completed = false;

public:
  MovingAverageModel(uint8_t samples = 10)
  {
    _samples = samples;
    _values = new uint16_t[_samples];
    reset();
  }

  ~MovingAverageModel()
  {
    delete[] _values;
  }

  void reset()
  {
    for (int i = 0; i < _samples; i++)
    {
      _values[i] = 0;
    }
    _index = 0;
    _sum = 0;
    _completed = false;
  }

  void addValue(uint16_t newValue)
  {
    uint16_t value = (newValue >= 0 && newValue <= UINT16_MAX) ? newValue : 0;

    _sum = _sum - _values[_index] + value;
    _values[_index] = value;

    if (!_completed && _index + 1 >= _samples)
    {
      _completed = true;
    }

    _index = (_index + 1) % _samples;
  }

  uint16_t value() const
  {
    return static_cast<uint16_t>(_sum / _samples);
  }

  bool isComplete()
  {
    return _completed;
  }
};

#endif