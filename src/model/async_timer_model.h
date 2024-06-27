#ifndef _ASYNC_TIMER_MODEL_
#define _ASYNC_TIMER_MODEL_

#include <Arduino.h>

#include "utils/time_format.h"

// max uint32_t = 4294967295
// max miliseconds = 4294967295
// max seconds = 4294967.295
// max minutes = 71582.78825
// max hours 1193.0464708333334
// max days = 49.710269618055555

class AsyncTimerModel {
  private:
  u_int32_t _startTime;
  u_int32_t _duration;
  bool _complete;

  public:
  AsyncTimerModel(bool startCompleted = false) : _complete(startCompleted) {
  }

  bool reset() {
    _complete = false;
    _startTime = esp_timer_get_time() / 1000;
    return _complete;
  }

  bool waitFor(u_int32_t durationInMs = 0) {
    if (!durationInMs) return true;
    if (_complete) return true;

    if (durationInMs != _duration) {
      _duration = durationInMs;
      reset();
    }

    u_int32_t currentTime = (esp_timer_get_time() / 1000);

    if (currentTime - _startTime >= _duration) {
      _complete = true;
    }

    return _complete;
  }

  void print() {
    Serial.print(F("duration: "));
    printFormattedTime(_duration);
    Serial.print(F(" remain: "));
    printFormattedTime(((_startTime + _duration) - (esp_timer_get_time() / 1000)));
    Serial.printf(" complete: %s", _complete ? "true" : "false");
    Serial.println("");
  }
};

#endif