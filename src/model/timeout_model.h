#ifndef _TIME_OUT_MODEL_
#define _TIME_OUT_MODEL_

#include <Arduino.h>

#include "utils/time_format.h"

// max uint32_t = 4294967295
// max miliseconds = 4294967295
// max seconds = 4294967.295
// max minutes = 71582.78825
// max hours 1193.0464708333334
// max days = 49.710269618055555

class TimeoutModel {
  private:
  u_int32_t _startTime;
  u_int32_t _duration;
  bool _running;

  public:
  TimeoutModel(uint32_t duration = 0) : _running(false) {
    _duration = duration;
  }

  void stop() {
    _running = false;
  }

  bool complete() {
    if (!_duration) return false;

    u_int32_t currentTime = (esp_timer_get_time() / 1000);
    if (!_running) {
      _running = true;
      _startTime = currentTime;
      return false;
    }

    if (currentTime - _startTime >= _duration) {
      _startTime = currentTime;
      return true;
    }

    return false;
  }

  void setDuration(uint32_t durationInMs) {
    if (durationInMs != _duration) {
      _duration = durationInMs;
      _startTime = (esp_timer_get_time() / 1000);
      _running = true;
    }
  }

  void print() {
    Serial.print(F("duration: "));
    printFormattedTime(_duration);
    Serial.print(F(" remain: "));
    printFormattedTime(((_startTime + _duration) - (esp_timer_get_time() / 1000)));
    Serial.println("");
  }
};

#endif