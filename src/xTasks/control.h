#ifndef _TASK_CONTROL_
#define _TASK_CONTROL_

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>

#include "global.h"

#define LEDC_CHANNEL 0                     // Canal PWM
#define LEDC_TIMER LEDC_TIMER_0            // Timer para o canal PWM
#define LEDC_MODE LEDC_HIGH_SPEED_MODE     // Modo de operação do timer
#define LEDC_RESOLUTION LEDC_TIMER_10_BIT  // Resolução do timer
#define LEDC_FREQUENCY 15000               // Frequência desejada em Hz
#define LEDC_PIN 15

void xTaskControl(void *parameter) {
  ledcSetup(LEDC_CHANNEL, LEDC_FREQUENCY, 10);
  ledcAttachPin(LEDC_PIN, LEDC_CHANNEL);

  while (1) {
    vTaskDelay(pdMS_TO_TICKS(100));
    digitalWrite(pins[1], HIGH);

    vTaskDelay(pdMS_TO_TICKS(100));
    digitalWrite(pins[1], LOW);

    digitalWrite(pins[3], temperatureFanEnabled.value());
    digitalWrite(pins[5], alarmEnabled.value());
    digitalWrite(pins[7], injectionScrewEnabled.value());

    ledcWrite(LEDC_CHANNEL, map(temperatureSetPoint.value(), 70, 160, 0, 1023));
  }
}

#endif