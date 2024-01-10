#ifndef _MODEL_DISPLAY_INT16_
#define _MODEL_DISPLAY_INT16_

#include <Preferences.h>

#include <stdint.h>
#include <freertos/semphr.h>

class DisplayInt16Model
{
private:
  SemaphoreHandle_t *_mutex;
  Preferences *_prefs;
  uint16_t _value;

public:
  uint16_t vp;
  DisplayInt16Model(uint16_t vpAddress, SemaphoreHandle_t *mutex, Preferences *prefs)
  {
    vp = vpAddress;
    _value = 0;
    _mutex = mutex;
    _prefs = prefs;
  }

  ~DisplayInt16Model()
  {
    vSemaphoreDelete(*_mutex);
  }

  void begin()
  {
    if (_prefs != nullptr)
    {
      _value = _prefs->getUInt(String(vp).c_str(), _value);
    }
  }

  uint16_t value()
  {
    return _value;
  }

  void setValue(uint16_t newValue)
  {
    if (xSemaphoreTake(*_mutex, portMAX_DELAY))
    {
      _value = newValue;
      if (_prefs != nullptr)
      {
        _prefs->putUInt(String(vp).c_str(), _value);
      }
      xSemaphoreGive(_mutex);
    }
  }
};

#endif