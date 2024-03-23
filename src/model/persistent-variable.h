#ifndef _PERSISTENT_VARIABLE_
#define _PERSISTENT_VARIABLE_

#include <Preferences.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

class PersistentVariable {
  private:
  SemaphoreHandle_t *mutex;

  uint16_t vp;
  int value;
  int minValue;
  int maxValue;

  public:
  PersistentVariable(uint16_t _vp, int _minValue, int _maxValue,
                     SemaphoreHandle_t *_mutex)
      : value(0), vp(_vp), minValue(_minValue), maxValue(_maxValue), mutex(_mutex) {
  }

  void retrieveValue(Preferences *prefs) {
    if (prefs != nullptr) {
      int restoredValue = prefs->getInt(std::to_string(vp).c_str(), minValue);
      value = constrain(restoredValue, minValue, maxValue);
    }
  }

  void setValue(int newValue) {
    int changedValue = constrain(newValue, minValue, maxValue);
    if (changedValue != value && xSemaphoreTake(*mutex, portMAX_DELAY) == pdTRUE) {
      value = changedValue;
      Preferences preferences;
      preferences.begin("VP", false);
      preferences.putInt(std::to_string(vp).c_str(), changedValue);
      preferences.end();
      xSemaphoreGive(*mutex);
    }
  }

  int getValue() {
    return value;
  }

  u_int16_t getVp() {
    return vp;
  }
};

#endif