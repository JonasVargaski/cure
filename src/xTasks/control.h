#ifndef _TASK_CONTROL_
#define _TASK_CONTROL_

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>

#include "global.h"
#include "model/timeout_model.h"

#define LEDC_CHANNEL 0                     // Canal PWM
#define LEDC_TIMER LEDC_TIMER_0            // Timer para o canal PWM
#define LEDC_MODE LEDC_HIGH_SPEED_MODE     // Modo de operação do timer
#define LEDC_RESOLUTION LEDC_TIMER_10_BIT  // Resolução do timer
#define LEDC_FREQUENCY 15000               // Frequência desejada em Hz
#define LEDC_PIN 15

// pin map output
#define alarmOutput 32
#define temperatureFanOutput 13
#define temperatureDamperAOutput 14
#define temperatureDamperBOutput 26
#define humidityDamperPwmOutput 15
#define humidityDamperA 27
#define humidityDamperB 25
#define injectionScrewA 12
#define injectionScrewB 33

void resetOutputs() {
  int pins[8] = {alarmOutput, temperatureFanOutput, temperatureDamperAOutput, temperatureDamperBOutput, humidityDamperA, humidityDamperB, injectionScrewA, injectionScrewB};
  for (int i = 0; i < 8; i++) {
    pinMode(pins[i], OUTPUT);
    digitalWrite(pins[i], LOW);
  }
}

void xTaskControl(void *parameter) {
  TimeoutModel toggleAlarmTimer;
  TimeoutModel resetAlarmTimer;

  ledcSetup(LEDC_CHANNEL, LEDC_FREQUENCY, 10);
  ledcAttachPin(LEDC_PIN, LEDC_CHANNEL);
  ledcWrite(LEDC_CHANNEL, 512);

  while (1) {
    // timers update
    resetAlarmTimer.setDuration(alarmReactiveParam.value() * 1000 * 60);
    toggleAlarmTimer.setDuration(900);
    // end timers update

    // Alarm handler control

    if (alarmEnabled.value()) {
      resetAlarmTimer.reset();

      if (toggleAlarmTimer.complete()) {
        alarmOutputState.setValueSync(!alarmOutputState.value(), false);
      }
    } else {
      if (toggleAlarmTimer.complete()) {
        alarmOutputState.setValueSync(LOW, false);
      }

      if (resetAlarmTimer.complete()) {
        alarmEnabled.setValueSync(true);
      }
    }

    // Changes logical state of outputs
    digitalWrite(alarmOutput, alarmOutputState.value());

    vTaskDelay(pdMS_TO_TICKS(20));
  }
}

#endif