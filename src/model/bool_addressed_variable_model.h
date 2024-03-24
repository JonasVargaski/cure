#ifndef _BOOL_ADDRESSED_VARIABLE_
#define _BOOL_ADDRESSED_VARIABLE_

#include "Arduino.h"
#include "model/addressed_variable_model.h"

class BoolStorageModel : public AddressedVariable {
  private:
  bool _value = false;

  public:
  BoolStorageModel(uint16_t address, SemaphoreHandle_t *mutex) : AddressedVariable(address, mutex) {
  }

  void setValue(bool value, bool saveInPreferences = true) {
    bool nextValue = value != 0;
    if (nextValue != _value) {
      _value = nextValue;
      this->resetTimeUpdate();
      if (saveInPreferences) {
        Preferences preferences;
        preferences.begin("VP", false);
        preferences.putBool(std::to_string(this->address()).c_str(), nextValue);
        preferences.end();
      }
    }
  }

  void setValueSync(bool value, bool saveInPreferences = true) {
    if (xSemaphoreTake(*_mutex, portMAX_DELAY) == pdTRUE) {
      setValue(value, saveInPreferences);
      xSemaphoreGive(*_mutex);
    }
  }

  bool loadValue(Preferences *prefs) {
    if (prefs != nullptr) {
      bool prefValue = prefs->getBool(std::to_string(this->address()).c_str(), false);
      setValue(prefValue, false);
    }
    return _value;
  }

  bool value() {
    return _value;
  }
};

#endif