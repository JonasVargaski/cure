#ifndef _UINT16_ADDRESSED_VARIABLE_
#define _UINT16_ADDRESSED_VARIABLE_

#include "Arduino.h"
#include "model/addressed_variable_model.h"

class Uint16StorageModel : public AddressedVariable {
  private:
  uint16_t _value = 0;
  uint16_t _min;
  uint16_t _max;

  public:
  Uint16StorageModel(uint16_t address, SemaphoreHandle_t *mutex, uint16_t min = 0, uint16_t max = UINT16_MAX) : AddressedVariable(address, mutex) {
    _max = max;
    _min = min;
  }

  void setValue(uint16_t value, bool saveInPreferences = true) {
    uint16_t nextValue = constrain(value, _min, _max);
    if (nextValue != _value) {
      _value = nextValue;
      this->resetTimeUpdate();
      if (saveInPreferences) {
        Preferences preferences;
        preferences.begin("VP", false);
        preferences.putUInt(std::to_string(this->address()).c_str(), nextValue);
        preferences.end();
      }
    }
  }

  void setValueSync(uint16_t value, bool saveInPreferences = true) {
    if (xSemaphoreTake(*_mutex, portMAX_DELAY) == pdTRUE) {
      setValue(value, saveInPreferences);
      xSemaphoreGive(*_mutex);
    }
  }

  uint16_t loadValue(Preferences *prefs) {
    if (prefs != nullptr) {
      setValue(prefs->getUInt(std::to_string(this->address()).c_str()), false);
    }
    return _value;
  }

  uint16_t value() {
    return _value;
  }
};

#endif