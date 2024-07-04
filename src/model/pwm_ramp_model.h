#ifndef _ACCELERATION_RAMP_PWM_MODEL_
#define _ACCELERATION_RAMP_PWM_MODEL_

#include <freertos/semphr.h>

#include "Arduino.h"

#define LEDC_TIMER LEDC_TIMER_0
#define LEDC_MODE LEDC_HIGH_SPEED_MODE

class PwmRampModel {
  private:
  int _channel;
  int _resolution;
  int _dutyCycleMax;

  uint32_t _startTime;
  bool _inProgress;

  public:
  PwmRampModel(int pin, int channel, int frequency, int resolutionBits = 10)
      : _channel(channel), _resolution(resolutionBits) {
    _inProgress = false;
    _dutyCycleMax = pow(2, _resolution) - 1;
    ledcSetup(channel, frequency, resolutionBits);
    ledcAttachPin(pin, _channel);
    ledcWrite(_channel, 0);
  }

  void start(int accelerationDurationMs, int maxDutyCycle = 100) {
    if (!_inProgress) {
      ledcWrite(_channel, 0);
      _startTime = esp_timer_get_time() / 1000;
      _inProgress = true;
    }

    if (_inProgress) {
      int dutyCycleMaxTarget = _dutyCycleMax * (maxDutyCycle / 100.0);
      int currentDutyCycle = ledcRead(_channel);
      if (currentDutyCycle >= dutyCycleMaxTarget) return;

      uint32_t elapsedTime = (esp_timer_get_time() / 1000) - _startTime;
      int nextDutyCycle = 0;
      if (elapsedTime < accelerationDurationMs) {
        int dutyCycle = map(elapsedTime, 0, accelerationDurationMs, 0, dutyCycleMaxTarget);
        nextDutyCycle = constrain(dutyCycle, 0, dutyCycleMaxTarget);
      } else {
        nextDutyCycle = dutyCycleMaxTarget;
      }

      ledcWrite(_channel, nextDutyCycle);
    }
  }

  void stop() {
    _inProgress = false;
    ledcWrite(_channel, 0);
  }
};

#endif