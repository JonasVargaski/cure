#ifndef _TASK_CONTROL_
#define _TASK_CONTROL_

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>

#include "enums.h"
#include "global.h"
#include "model/acceleration_ramp_pwm_model.h"
#include "model/on_off_timer_model.h"
#include "model/timeout_model.h"

#define ACCELERATION_RAMP_IN_MS 600

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
#define humidityDamperPwm 15

void resetOutputs() {
  int pins[9] = {alarmOutput, temperatureFanOutput, temperatureDamperAOutput, temperatureDamperBOutput, humidityDamperA, humidityDamperB, injectionMachineA, injectionMachineB, humidityDamperPwm};
  for (int i = 0; i < 8; i++) {
    pinMode(pins[i], OUTPUT);
    digitalWrite(pins[i], LOW);
  }
}

void xTaskControl(void *parameter) {
  TimeoutModel toggleAlarmTimer(900);
  TimeoutModel reactiveAlarmTimer;
  TimeoutModel intervalFanTimer;
  TimeoutModel debug(1000);

  OnOffTimerModel humidityDamperTimer;
  AccelerationRampPwmModel humidityDamperOutputRamp(15, 0, 1500);

  while (!temperatureSensor.complete() || !humiditySensor.complete()) {
    vTaskDelay(pdMS_TO_TICKS(100));
  }

  while (1) {
    vTaskDelay(pdMS_TO_TICKS(15));

#pragma region CONFIGURE_TIMERS
    reactiveAlarmTimer.setDuration(alarmReactiveParam.value() * 1000 * 60);
    intervalFanTimer.setDuration(temperatureFanIntervalParam.value());

#pragma endregion

#pragma region HUMIDITY_DAMPER
    int damperDirState = eHumidityDamperStatus::DAMPER_OFF;

    if (humiditySensor.value() - humiditySetPoint.value() >= humidityDamperDiffParam.value()) {
      humidityDamperTimer.setDuration(humidityDamperEnableTimeParam.value(), humidityDamperDisableTimeParam.value() * 1000, false);
      damperDirState = humidityDamperTimer.isEnabledNow() ? eHumidityDamperStatus::DAMPER_OPEN : eHumidityDamperStatus::DAMPER_OFF;
    } else {
      humidityDamperTimer.setDuration(35000, 60000);  // 35s ON, 60s OFF
      damperDirState = humidityDamperTimer.isEnabledNow() ? eHumidityDamperStatus::DAMPER_CLOSE : eHumidityDamperStatus::DAMPER_OFF;
    }

    digitalWrite(humidityDamperA, damperDirState == eHumidityDamperStatus::DAMPER_OFF ? LOW : damperDirState != eHumidityDamperStatus::DAMPER_OPEN);
    digitalWrite(humidityDamperB, damperDirState == eHumidityDamperStatus::DAMPER_OFF ? LOW : damperDirState == eHumidityDamperStatus::DAMPER_OPEN);
    humidityDamperOutputState.setValueSync(damperDirState, false);

    if (damperDirState != eHumidityDamperStatus::DAMPER_OFF) {
      int timeEnabled = humidityDamperEnableTimeParam.value();
      humidityDamperOutputRamp.start(constrain(timeEnabled, timeEnabled, ACCELERATION_RAMP_IN_MS));
    } else
      humidityDamperOutputRamp.stop();

#pragma endregion

#pragma region TEMPERATURE_FAN_INJECTION // TODO
    bool shouldActivateFan = false;

    if (shouldActivateFan && intervalFanTimer.complete())
      digitalWrite(temperatureFanOutput, HIGH);
    else if (!shouldActivateFan) {
      digitalWrite(temperatureFanOutput, LOW);
    }

    temperatureFanOutputState.setValueSync(digitalRead(temperatureFanOutput), false);
#pragma endregion

#pragma region ALARMS
    alarmFlags.VENTILATION_FAILURE = true;  // TODO: read input
    alarmFlags.ELECTRICAL_SUPPLY = true;    // TODO: read input

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
    bool shouldActivateAlarm = false;

    if (alarmFlags.VENTILATION_FAILURE && alarmVentilationTypeParam.value())
      shouldActivateAlarm = true;
    else if (alarmFlags.ELECTRICAL_SUPPLY && alarmVentilationTypeParam.value())
      shouldActivateAlarm = true;
    else if (alarmFlags.TEMPERATURE_HIGH && (alarmTemperatureTypeParam.value() == eAlarmEnableType::HIGH_VALUE || alarmTemperatureTypeParam.value() == eAlarmEnableType::ALL))
      shouldActivateAlarm = true;
    else if (alarmFlags.TEMPERATURE_LOW && (alarmTemperatureTypeParam.value() == eAlarmEnableType::LOW_VALUE || alarmTemperatureTypeParam.value() == eAlarmEnableType::ALL))
      shouldActivateAlarm = true;
    else if (alarmFlags.HUMIDITY_HIGH && (alarmHumidityTypeParam.value() == eAlarmEnableType::HIGH_VALUE || alarmHumidityTypeParam.value() == eAlarmEnableType::ALL))
      shouldActivateAlarm = true;
    else if (alarmFlags.HUMIDITY_LOW && (alarmHumidityTypeParam.value() == eAlarmEnableType::LOW_VALUE || alarmHumidityTypeParam.value() == eAlarmEnableType::ALL))
      shouldActivateAlarm = true;
    else if (alarmFlags.SECURITY_MODE_HIGH && (alarmSecurityTypeParam.value() == eAlarmEnableType::HIGH_VALUE || alarmSecurityTypeParam.value() == eAlarmEnableType::ALL))
      shouldActivateAlarm = true;
    else if (alarmFlags.SECURITY_MODE_LOW && (alarmSecurityTypeParam.value() == eAlarmEnableType::LOW_VALUE || alarmSecurityTypeParam.value() == eAlarmEnableType::ALL))
      shouldActivateAlarm = true;

    if (alarmEnabled.value()) {
      reactiveAlarmTimer.stop();
    } else if (reactiveAlarmTimer.complete()) {
      alarmEnabled.setValueSync(true);
    }

    if (toggleAlarmTimer.complete()) {
      digitalWrite(alarmOutput, shouldActivateAlarm && alarmEnabled.value() ? !digitalRead(alarmOutput) : LOW);
      alarmOutputState.setValueSync(digitalRead(alarmOutput), false);
    }

#pragma endregion

#pragma region DEBUG
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

      Serial.printf("[ALARM] enabled: %d, shouldActivateAlarm: %d - ", alarmEnabled.value(), shouldActivateAlarm);
      Serial.println(alarms);
      Serial.printf("[HUMIDITY] damperAction: %s\n", damperDirState == eHumidityDamperStatus::DAMPER_OFF ? "OFF" : damperDirState == eHumidityDamperStatus::DAMPER_OPEN ? "OPEN"
                                                                                                                                                                        : "CLOSE");

      Serial.println(F("---------------------------------------------------------"));
    }
#pragma endregion
  }
}

#endif