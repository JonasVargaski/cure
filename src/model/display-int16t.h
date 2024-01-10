#ifndef _MODEL_DISPLAY_INT16_
#define _MODEL_DISPLAY_INT16_

#include <stdint.h>
#include <freertos/semphr.h>
#include <Preferences.h>

class DisplayInt16Model
{
private:
  SemaphoreHandle_t _mutex;
  uint16_t _value;
  bool _persist = false;

public:
  uint16_t vp;
  DisplayInt16Model(uint16_t vpAddress, bool persist)
  {
    vp = vpAddress;
    _value = 0;
    _mutex = xSemaphoreCreateMutex();
    _persist = persist;

    if (persist)
    {
      Preferences preferences;
      preferences.begin("vp_prefs", false);
      _value = preferences.getUInt(String(vp).c_str(), _value);
      preferences.end();
    }
  }

  ~DisplayInt16Model()
  {
    vSemaphoreDelete(_mutex);
  }

  uint16_t value()
  {
    return _value;
  }

  void setValue(uint16_t newValue)
  {
    if (_mutex && xSemaphoreTake(_mutex, portMAX_DELAY))
    {
      _value = newValue;
      Preferences preferences;
      preferences.begin("vp_prefs", false);
      preferences.putUInt(String(vp).c_str(), _value);
      preferences.end();
      xSemaphoreGive(_mutex);
    }
  }
};

#endif