#ifndef _TEXT_ADDRESSED_VARIABLE_
#define _TEXT_ADDRESSED_VARIABLE_

#include "Arduino.h"
#include "model/addressed_variable_model.h"

class TextStorageModel : public AddressedVariable {
  private:
  char* _value;
  unsigned int _size;

  public:
  TextStorageModel(uint16_t address, SemaphoreHandle_t* mutex, unsigned int size) : AddressedVariable(address, mutex) {
    _size = size + 1;
    _value = new char[size];
  }

  ~TextStorageModel() {
    delete[] _value;
  }

  void setValue(const char* value, bool saveInPreferences = true) {
    if (strcmp(value, _value) != 0) {
      strncpy(_value, value, _size - 1);
      _value[_size - 1] = '\0';
      if (saveInPreferences) {
        Preferences preferences;
        preferences.begin("VP", false);
        preferences.putString(std::to_string(this->address()).c_str(), _value);
        preferences.end();
      }
    }
  }

  void setValueSync(const char* value, bool saveInPreferences = true) {
    if (xSemaphoreTake(*_mutex, portMAX_DELAY) == pdTRUE) {
      setValue(value, saveInPreferences);
      xSemaphoreGive(*_mutex);
    }
  }

  const char* loadValue(Preferences* prefs) {
    if (prefs != nullptr) {
      String stringValue = prefs->getString(std::to_string(this->address()).c_str(), "");
      stringValue.toCharArray(_value, _size);
    }
    return _value;
  }

  const char* value() const {
    return _value;
  }
};

#endif