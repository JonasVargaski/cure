#include <Arduino.h>

class TimeoutModel {
  private:
  TickType_t _startTime;
  unsigned int _duration;
  bool _running = true;

  public:
  TimeoutModel(unsigned int duration = 0) : _running(false) {
    _duration = constrain(duration, 0, INT_MAX);
  }

  void reset() {
    if (_duration > 0) {
      _startTime = xTaskGetTickCount();
      _running = true;
    } else {
      _running = false;
    }
  }

  bool complete() {
    if (_duration < 1) return false;

    TickType_t currentTime = xTaskGetTickCount();
    TickType_t elapsedTime = currentTime - _startTime;
    _running = !(elapsedTime >= pdMS_TO_TICKS(_duration));
    if (!_running) _startTime = xTaskGetTickCount();
    return !_running;
  }

  void setDuration(unsigned int durationInMs) {
    if (durationInMs != _duration) {
      _duration = durationInMs;
      _startTime = xTaskGetTickCount();
      _running = true;
    }
  }
};