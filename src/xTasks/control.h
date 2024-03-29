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
#define injectionMachineA 12
#define injectionMachineB 33

void resetOutputs() {
  int pins[8] = {alarmOutput, temperatureFanOutput, temperatureDamperAOutput, temperatureDamperBOutput, humidityDamperA, humidityDamperB, injectionMachineA, injectionMachineB};
  pinMode(15, OUTPUT);
  digitalWrite(15, LOW);
  for (int i = 0; i < 8; i++) {
    pinMode(pins[i], OUTPUT);
    digitalWrite(pins[i], LOW);
  }
}

void xTaskControl(void *parameter) {
  TimeoutModel toggleAlarmTimer;
  TimeoutModel reactiveAlarmTimer;
  TimeoutModel debug(1000);

  ledcSetup(LEDC_CHANNEL, LEDC_FREQUENCY, 10);
  ledcAttachPin(LEDC_PIN, LEDC_CHANNEL);
  ledcWrite(LEDC_CHANNEL, 120);

  while (!temperatureSensor.complete() || !humiditySensor.complete()) {
    vTaskDelay(pdMS_TO_TICKS(100));
  }

  while (1) {
#pragma region TIMERS
    reactiveAlarmTimer.setDuration(alarmReactiveParam.value() * 1000 * 60);
    toggleAlarmTimer.setDuration(900);
#pragma endregion

#pragma region ALARMS

    alarmFlags.VENTILATION_FAILURE = false;  // TODO: read input
    alarmFlags.ELECTRICAL_SUPPLY = false;    // TODO: read input

    alarmFlags.TEMPERATURE_SENSOR_FAILURE = temperatureSensor.value() < 1;
    alarmFlags.HUMIDITY_SENSOR_FAILURE = humiditySensor.value() < 1;

    if (temperatureSensor.value() - temperatureSetPoint.value() >= securityModeTemperatureDiffParam.value()) {
      alarmFlags.SECURITY_MODE_LOW = false;
      alarmFlags.SECURITY_MODE_HIGH = true;
    } else if (temperatureSetPoint.value() - temperatureSensor.value() >= securityModeTemperatureDiffParam.value()) {
      alarmFlags.SECURITY_MODE_LOW = true;
      alarmFlags.SECURITY_MODE_HIGH = false;
    } else if (abs(temperatureSensor.value() - temperatureSetPoint.value()) < securityModeTemperatureDiffParam.value() - 1) {
      alarmFlags.SECURITY_MODE_LOW = false;
      alarmFlags.SECURITY_MODE_HIGH = false;
    }

    if (temperatureSensor.value() - temperatureSetPoint.value() >= alarmTemperatureDiffParam.value()) {
      alarmFlags.TEMPERATURE_LOW = false;
      alarmFlags.TEMPERATURE_HIGH = true;
    } else if (temperatureSetPoint.value() - temperatureSensor.value() >= alarmTemperatureDiffParam.value()) {
      alarmFlags.TEMPERATURE_LOW = true;
      alarmFlags.TEMPERATURE_HIGH = false;
    } else if (abs(temperatureSensor.value() - temperatureSetPoint.value()) < alarmTemperatureDiffParam.value() - 1) {
      alarmFlags.TEMPERATURE_LOW = false;
      alarmFlags.TEMPERATURE_HIGH = false;
    }

    if (humiditySensor.value() - humiditySetPoint.value() >= alarmHumidityDiffParam.value()) {
      alarmFlags.HUMIDITY_LOW = false;
      alarmFlags.HUMIDITY_HIGH = true;
    } else if (humiditySetPoint.value() - humiditySensor.value() >= alarmHumidityDiffParam.value()) {
      alarmFlags.HUMIDITY_LOW = true;
      alarmFlags.HUMIDITY_HIGH = false;
    } else if (abs(humiditySensor.value() - humiditySetPoint.value()) < alarmHumidityDiffParam.value() - 1) {
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
    else if (alarmFlags.SECURITY_MODE_HIGH && (alarmSecurityTypeParam.value() == eAlarmEnableType::HIGH_VALUE || alarmSecurityTypeParam.value() == eAlarmEnableType::ALL))
      shouldAlarm = true;
    else if (alarmFlags.SECURITY_MODE_LOW && (alarmSecurityTypeParam.value() == eAlarmEnableType::LOW_VALUE || alarmSecurityTypeParam.value() == eAlarmEnableType::ALL))
      shouldAlarm = true;

    if (alarmEnabled.value()) {
      reactiveAlarmTimer.stop();
    } else if (reactiveAlarmTimer.complete()) {
      alarmEnabled.setValueSync(true);
    }

    if (toggleAlarmTimer.complete()) {
      digitalWrite(alarmOutput, shouldAlarm && alarmEnabled.value() ? !digitalRead(alarmOutput) : LOW);
      alarmOutputState.setValueSync(digitalRead(alarmOutput), false);
    }

#pragma endregion

    vTaskDelay(pdMS_TO_TICKS(20));

    /////////////////////////////////////////// DEBUG //////////////////////////////////////////////////////

    if (debug.complete()) {
      String alarms = "";
      if (alarmFlags.ELECTRICAL_SUPPLY) alarms.concat("ELECTRICAL_SUPPLY ");
      if (alarmFlags.VENTILATION_FAILURE) alarms.concat("VENTILATION_FAILURE ");
      if (alarmFlags.TEMPERATURE_SENSOR_FAILURE) alarms.concat("TEMPERATURE_SENSOR_FAILURE ");
      if (alarmFlags.HUMIDITY_SENSOR_FAILURE) alarms.concat("HUMIDITY_SENSOR_FAILURE ");
      if (alarmFlags.SECURITY_MODE_LOW) alarms.concat("SECURITY_MODE_LOW ");
      if (alarmFlags.SECURITY_MODE_HIGH) alarms.concat("SECURITY_MODE_HIGH ");
      if (alarmFlags.TEMPERATURE_HIGH) alarms.concat("TEMPERATURE_HIGH ");
      if (alarmFlags.TEMPERATURE_LOW) alarms.concat("TEMPERATURE_LOW ");
      if (alarmFlags.HUMIDITY_HIGH) alarms.concat("HUMIDITY_HIGH ");
      if (alarmFlags.HUMIDITY_LOW) alarms.concat("HUMIDITY_LOW ");

      Serial.printf("[ALARM] enabled: %d, shouldAlarm: %d - ", alarmEnabled.value(), shouldAlarm);
      Serial.println(alarms);
    }
  }
}

#endif