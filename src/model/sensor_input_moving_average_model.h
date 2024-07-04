#ifndef _SENSOR_INPUT_MOVING_AVERAGE_ANALOG_
#define _SENSOR_INPUT_MOVING_AVERAGE_ANALOG_

#include <stdint.h>

#include "Arduino.h"
#include "model/addressed_variable_model.h"

class SensorInputAverageModel : public AddressedVariable {
  private:
  uint8_t _samples;
  int *_values;
  int _index;
  int _sum;
  bool _completed = false;
  unsigned int _maxRangeValue;
  int _minRangeValue;

  std::function<int(uint16_t)> _conversionCallback;

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
    _values = new int[_samples];
    _minRangeValue = (INT32_MIN + 1);
    _maxRangeValue = (INT32_MAX - 1);
    reset();

    _conversionCallback = [](uint16_t value) {
      return static_cast<int>(value);
    };
  }

  ~SensorInputAverageModel() {
    delete[] _values;
  }

  void setConversionCallback(std::function<int(uint16_t)> conversionCallback) {
    _conversionCallback = conversionCallback;
  }

  void setValidRange(int min, unsigned int max) {
    _minRangeValue = min;
    _maxRangeValue = max;
  }

  void addValue(uint16_t newValue) {
    if (xSemaphoreTake(*_mutex, portMAX_DELAY) == pdTRUE) {
      int parsedValue = _conversionCallback(newValue);

      _sum = _sum - _values[_index] + parsedValue;
      _values[_index] = parsedValue;

      if (!_completed && _index + 1 >= _samples) {
        _completed = true;
      }

      _index = (_index + 1) % _samples;
      this->resetTimeUpdate();
      xSemaphoreGive(*_mutex);
    }
  }

  int value() const {
    return (_sum / _samples);
  }

  bool complete() {
    return _completed;
  }

  bool isOutOfRange() {
    int currentValue = (_sum / _samples);
    return currentValue <= _minRangeValue || currentValue >= _maxRangeValue;
  }
};

#endif