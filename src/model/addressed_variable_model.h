#ifndef _ADDRESSED_VARIABLE_
#define _ADDRESSED_VARIABLE_

#include <Preferences.h>
#include <freertos/semphr.h>

#include "Arduino.h"

class AddressedVariable {
  private:
  uint16_t _address;
  TickType_t _interval;
  TickType_t _lastUpdatedDisplay;

  protected:
  SemaphoreHandle_t *_mutex;

  public:
  AddressedVariable(uint16_t address, SemaphoreHandle_t *mutex = nullptr) : _address(address), _mutex(mutex) {
    _interval = pdMS_TO_TICKS(0);
  }

  void resetTimeUpdate() {
    _lastUpdatedDisplay = 0;
  }

  bool shouldUpdateDisplay() {
    TickType_t currentTime = xTaskGetTickCount();
    bool shouldUpdate = currentTime - _lastUpdatedDisplay >= _interval;
    if (shouldUpdate) {
      _lastUpdatedDisplay = currentTime;
      _interval = pdMS_TO_TICKS(random(1000, 5000));
    }
    return shouldUpdate;
  }

  uint16_t address() {
    return _address;
  }
};

#endif