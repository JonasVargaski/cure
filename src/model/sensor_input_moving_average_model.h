#ifndef _SENSOR_INPUT_MOVING_AVERAGE_ANALOG_
#define _SENSOR_INPUT_MOVING_AVERAGE_ANALOG_

#include <stdint.h>

#include "Arduino.h"
#include "model/addressed_variable_model.h"

class SensorInputAverageModel : public AddressedVariable {
  private:
  uint8_t _samples;
  float *_values;
  int _index;
  float _sum;
  bool _completed = false;

  std::function<float(uint16_t)> _conversionCallback;

  void reset() {
    for (int i = 0; i < _samples; i++) {
      _values[i] = 0;
    }
    _index = 0;
    _sum = 0;
    _completed = false;
  }

  public:
  SensorInputAverageModel(u_int16_t address, SemaphoreHandle_t *mutex, uint8_t samples) : AddressedVariable(address, mutex) {
    _samples = samples;
    _values = new float[_samples];
    reset();

    _conversionCallback = [](uint16_t value) {
      return static_cast<float>(value);
    };
  }

  ~SensorInputAverageModel() {
    delete[] _values;
  }

  void setConversionCallback(std::function<float(uint16_t)> conversionCallback) {
    _conversionCallback = conversionCallback;
  }

  void addValue(uint16_t newValue) {
    if (xSemaphoreTake(*_mutex, portMAX_DELAY) == pdTRUE) {
      float parsedValue = _conversionCallback(newValue);
      float nextValue = constrain(parsedValue, 0, UINT16_MAX);

      _sum = _sum - _values[_index] + nextValue;
      _values[_index] = nextValue;

      if (!_completed && _index + 1 >= _samples) {
        _completed = true;
      }

      _index = (_index + 1) % _samples;
      xSemaphoreGive(*_mutex);
    }
  }

  float value() const {
    return (_sum / _samples);
  }

  bool isComplete() {
    return _completed;
  }
};

#endif