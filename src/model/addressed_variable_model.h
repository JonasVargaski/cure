#ifndef _ADDRESSED_VARIABLE_
#define _ADDRESSED_VARIABLE_

#include <Preferences.h>
#include <freertos/semphr.h>

#include "Arduino.h"

class AddressedVariable {
  private:
  uint16_t _address;

  protected:
  SemaphoreHandle_t *_mutex;

  public:
  AddressedVariable(uint16_t address, SemaphoreHandle_t *mutex = nullptr) : _address(address), _mutex(mutex) {}

  uint16_t address() {
    return _address;
  }
};

#endif