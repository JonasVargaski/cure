#ifndef _ACCELERATION_RAMP_PWM_MODEL_
#define _ACCELERATION_RAMP_PWM_MODEL_

#include <freertos/semphr.h>

#include "Arduino.h"

#define LEDC_TIMER LEDC_TIMER_0
#define LEDC_MODE LEDC_HIGH_SPEED_MODE

class PwmRampModel {
  private:
  int channel;
  int resolution;
  int dutyCycleMax;

  uint32_t startTime;
  bool inProgress;

  public:
  PwmRampModel(int pin, int channel, int frequency, int resolutionBits = 10)
      : channel(channel), resolution(resolutionBits) {
    inProgress = false;
    dutyCycleMax = pow(2, resolution) - 1;
    ledcSetup(channel, frequency, resolutionBits);
    ledcAttachPin(pin, channel);
    ledcWrite(channel, 0);
  }

  void start(int accelerationDurationMs, int maxDutyCyclePercent = 100) {
    if (!inProgress) {
      ledcWrite(channel, 0);
      startTime = esp_timer_get_time() / 1000;
      inProgress = true;
    }

    if (inProgress) {
      int dutyCycleMaxTarget = dutyCycleMax * (maxDutyCyclePercent / 100.0);
      int currentDutyCycle = ledcRead(channel);
      if (currentDutyCycle >= dutyCycleMaxTarget) return;

      uint32_t elapsedTime = (esp_timer_get_time() / 1000) - startTime;
      int nextDutyCycle = 0;
      if (elapsedTime < accelerationDurationMs) {
        int dutyCycle = map(elapsedTime, 0, accelerationDurationMs, 0, dutyCycleMaxTarget);
        nextDutyCycle = constrain(dutyCycle, 0, dutyCycleMaxTarget);
      } else {
        nextDutyCycle = dutyCycleMaxTarget;
      }

      ledcWrite(channel, nextDutyCycle);
    }
  }

  void stop() {
    inProgress = false;
    ledcWrite(channel, 0);
  }
};

#endif