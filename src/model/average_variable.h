#ifndef AverageVariable_h
#define AverageVariable_h

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

class AverageVariable {
  private:
  SemaphoreHandle_t *mutex;
  uint8_t samples;
  uint8_t index;
  int32_t sum;
  int rangeMin;
  int rangeMax;
  int *values;
  bool completed;

  void reset() {
    memset(values, 0, samples * sizeof(int32_t));
    index = 0;
    sum = 0;
    value = 0;
    completed = false;
  }

  public:
  const uint16_t address;
  int value;

  AverageVariable(uint16_t addr, SemaphoreHandle_t *mtx, uint8_t samples = 10) : address(addr), mutex(mtx), samples(samples), value(0) {
    values = new int[samples];
    rangeMin = INT_MIN;
    rangeMax = INT_MAX;
    reset();
  }

  ~AverageVariable() {
    delete[] values;
  }

  void setValue(const int newValue) {
    if (xSemaphoreTake(*mutex, portMAX_DELAY) == pdTRUE) {
      sum = sum - values[index] + newValue;
      values[index] = newValue;
      index = (index + 1) % samples;
      value = sum / samples;
      if (!completed && index == 0) completed = true;
      xSemaphoreGive(*mutex);
    }
  }

  void setRange(int min, int max) {
    if (min == rangeMin && max == rangeMax) return;
    rangeMin = min;
    rangeMax = max;
    reset();
  }

  bool isOutOfRange() {
    return value <= rangeMin || value >= rangeMax;
  }

  bool isCompleted() {
    return completed;
  }
};

#endif