#ifndef _ON_OFF_TIMER_MODEL_
#define _ON_OFF_TIMER_MODEL_

#include <Arduino.h>

class CyclicTimerModel {
  private:
  u_int32_t _startTime;
  u_int32_t _on_duration;
  u_int32_t _off_duration;
  bool _enabled;

  public:
  CyclicTimerModel() {
    _on_duration = 0;
    _off_duration = 0;
    _enabled = false;
  }

  void reset(bool startEnabled = true) {
    _startTime = (esp_timer_get_time() / 1000);
    _enabled = startEnabled;
  }

  void setDuration(uint32_t durationOnInMs, uint32_t durationOffInMs, bool startEnabled = true) {
    if (durationOnInMs != _on_duration || durationOffInMs != _off_duration) {
      _on_duration = durationOnInMs;
      _off_duration = durationOffInMs;
      _enabled = startEnabled;
      _startTime = (esp_timer_get_time() / 1000);
    }
  }

  bool isEnabledNow() {
    uint32_t currentTime = (esp_timer_get_time() / 1000);

    if (_enabled) {
      if (currentTime - _startTime >= _on_duration) {
        _startTime = currentTime;
        _enabled = false;
      }
    } else {
      if (currentTime - _startTime >= _off_duration) {
        _startTime = currentTime;
        _enabled = true;
      }
    }

    return _enabled;
  }
};

#endif