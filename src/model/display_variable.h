#ifndef DisplayVariable_h
#define DisplayVariable_h

#include <Arduino.h>
#include <Preferences.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

class DisplayInt {
  private:
  SemaphoreHandle_t *mutex;
  int rangeMin;
  int rangeMax;

  public:
  int value;
  const uint16_t address;

  DisplayInt(uint16_t addr, SemaphoreHandle_t *mtx, int min, int max) : address(addr), mutex(mtx), value(-1), rangeMin(min), rangeMax(max) {}

  void setValue(const int newValue, const bool persist = true) {
    if (xSemaphoreTake(*mutex, portMAX_DELAY) == pdTRUE) {
      int rangeValue = constrain(newValue, rangeMin, rangeMax);
      if (value != rangeValue) {
        value = rangeValue;
        if (persist) {
          Preferences preferences;
          preferences.begin("vp", false);
          preferences.putInt(String(address).c_str(), rangeValue);
          preferences.end();
        }
      }
      xSemaphoreGive(*mutex);
    }
  }

  void setRange(int min, int max) {
    if (min == rangeMin && max == rangeMax) return;
    rangeMin = min;
    rangeMax = max;
    setValue(value);
  }
};

class DisplayText {
  private:
  SemaphoreHandle_t *mutex;
  unsigned int size;

  public:
  char *value;
  const uint16_t address;

  DisplayText(uint16_t addr, SemaphoreHandle_t *mtx, unsigned int length) : address(addr), mutex(mtx) {
    size = length + 1;
    value = value = new char[size];
    memset(value, 0, size);
  }

  ~DisplayText() {
    delete[] value;
  }

  void setValue(const char *newValue, const bool persist = true) {
    if (xSemaphoreTake(*mutex, portMAX_DELAY) == pdTRUE) {
      if (strcmp(newValue, value) != 0) {
        Serial.println("update text value");
        strncpy(value, newValue, size - 1);
        value[size - 1] = '\0';
        if (persist) {
          Preferences preferences;
          preferences.begin("vp", false);
          preferences.putString(String(address).c_str(), value);
          preferences.end();
        }
      }
      xSemaphoreGive(*mutex);
    }
  }
};

#endif