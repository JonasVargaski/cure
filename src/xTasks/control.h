#ifndef _TASK_CONTROL_
#define _TASK_CONTROL_

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>

#include "enums.h"
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

    // Definition alarm section

    alarmFlags.SECURITY_MODE_HI = false;     // TODO: implement logic
    alarmFlags.SECURITY_MODE_LOW = false;    // TODO: implement logic
    alarmFlags.VENTILATION_FAILURE = false;  // TODO: read input
    alarmFlags.ELECTRICAL_SUPPLY = false;    // TODO: read input

    alarmFlags.TEMPERATURE_SENSOR_FAILURE = temperatureSensor.value() < 1;
    alarmFlags.HUMIDITY_SENSOR_FAILURE = humiditySensor.value() < 1;

    if ((temperatureSensor.value() - alarmTemperatureDiffParam.value()) > temperatureSetPoint.value()) {
      alarmFlags.TEMPERATURE_LOW = false;
      alarmFlags.TEMPERATURE_HIGH = true;
    } else if ((temperatureSensor.value() + alarmTemperatureDiffParam.value()) < temperatureSetPoint.value()) {
      alarmFlags.TEMPERATURE_LOW = true;
      alarmFlags.TEMPERATURE_HIGH = false;
    } else if ((alarmFlags.TEMPERATURE_LOW || alarmFlags.TEMPERATURE_HIGH) && abs(temperatureSensor.value() - alarmTemperatureDiffParam.value() - temperatureSetPoint.value()) <= 2) {
      alarmFlags.TEMPERATURE_LOW = false;
      alarmFlags.TEMPERATURE_HIGH = false;
    }

    if ((humiditySensor.value() - alarmHumidityDiffParam.value()) > humiditySetPoint.value()) {
      alarmFlags.HUMIDITY_LOW = false;
      alarmFlags.HUMIDITY_HIGH = true;
    } else if ((humiditySensor.value() + alarmHumidityDiffParam.value()) < humiditySetPoint.value()) {
      alarmFlags.HUMIDITY_LOW = true;
      alarmFlags.HUMIDITY_HIGH = false;
    } else if ((alarmFlags.HUMIDITY_LOW || alarmFlags.HUMIDITY_HIGH) && abs(humiditySensor.value() - alarmHumidityDiffParam.value() - humiditySetPoint.value()) <= 2) {
      alarmFlags.HUMIDITY_LOW = false;
      alarmFlags.HUMIDITY_HIGH = false;
    }

    // End definition alarms
    bool shouldAlarm = false;

    if (alarmFlags.VENTILATION_FAILURE && alarmVentilationTypeParam.value())
      shouldAlarm = true;
    else if (alarmFlags.ELECTRICAL_SUPPLY && alarmVentilationTypeParam.value())
      shouldAlarm = true;
    else if (alarmFlags.TEMPERATURE_HIGH && (alarmTemperatureTypeParam.value() == eAlarmEnableType::HIGH_VALUE || alarmTemperatureTypeParam.value() == eAlarmEnableType::ALL))
      shouldAlarm = true;
    else if (alarmFlags.TEMPERATURE_LOW && (alarmTemperatureTypeParam.value() == eAlarmEnableType::LOW_VALUE || alarmTemperatureTypeParam.value() == eAlarmEnableType::ALL))
      shouldAlarm = true;
    else if (alarmFlags.HUMIDITY_HIGH && (alarmHumidityTypeParam.value() == eAlarmEnableType::HIGH_VALUE || alarmHumidityTypeParam.value() == eAlarmEnableType::ALL))
      shouldAlarm = true;
    else if (alarmFlags.HUMIDITY_LOW && (alarmHumidityTypeParam.value() == eAlarmEnableType::LOW_VALUE || alarmHumidityTypeParam.value() == eAlarmEnableType::ALL))
      shouldAlarm = true;
    else if (alarmFlags.SECURITY_MODE_HI && (alarmSecurityTypeParam.value() == eAlarmEnableType::HIGH_VALUE || alarmSecurityTypeParam.value() == eAlarmEnableType::ALL))
      shouldAlarm = true;
    else if (alarmFlags.SECURITY_MODE_LOW && (alarmSecurityTypeParam.value() == eAlarmEnableType::LOW_VALUE || alarmSecurityTypeParam.value() == eAlarmEnableType::ALL))
      shouldAlarm = true;

    if (shouldAlarm && alarmEnabled.value()) {
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