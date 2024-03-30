#ifndef _TASK_CONTROL_
#define _TASK_CONTROL_

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>

#include "enums.h"
#include "global.h"
#include "model/acceleration_ramp_pwm_model.h"
#include "model/async_timer_model.h"
#include "model/cyclic_timer_model.h"

#define MIN_ACCELERATION_RAMP_IN_MS 700

// pin map output
#define alarmOutput 32
#define temperatureFanOutput 13
#define temperatureDamperAOutput 14
#define temperatureDamperBOutput 26
#define humidityDamperPwmOutput 15
#define humidityDamperA 27
#define humidityDamperB 25
#define humidityDamperPwm 15
#define injectionMachineA 12
#define injectionMachineB 33
#define electricalInputFlag 34
#define ventilationInputFlag 35

void resetIOs() {
  int pins[9] = {alarmOutput, temperatureFanOutput, temperatureDamperAOutput, temperatureDamperBOutput, humidityDamperA, humidityDamperB, injectionMachineA, injectionMachineB, humidityDamperPwm};
  for (int i = 0; i < 8; i++) {
    pinMode(pins[i], OUTPUT);
    digitalWrite(pins[i], LOW);
  }
  pinMode(electricalInputFlag, INPUT_PULLUP);
  pinMode(ventilationInputFlag, INPUT_PULLUP);
}

void xTaskControl(void *parameter) {
  AsyncTimerModel debug;

  AsyncTimerModel toggleAlarmTimer;
  AsyncTimerModel reactiveAlarmTimer;
  AsyncTimerModel reactiveFanTimer;
  AsyncTimerModel intervalFanStateTimer;
  AsyncTimerModel intervalInjectMachineStateTimer;
  AsyncTimerModel injectionMachineClearTimer;

  CyclicTimerModel humidityDamperOnOffTimer;
  CyclicTimerModel injectionMachineOnOffTimer;

  AccelerationRampPwmModel humidityDamperOutputRamp(humidityDamperPwmOutput, 0, 1500);

  while (!temperatureSensor.complete() || !humiditySensor.complete()) {
    vTaskDelay(pdMS_TO_TICKS(100));
  }

  while (1) {
    vTaskDelay(pdMS_TO_TICKS(15));

#pragma region INPUT_FLAGS
    bool hasVentilationFail = !digitalRead(ventilationInputFlag);
    bool hasElectricalFail = !digitalRead(electricalInputFlag);
#pragma endregion

#pragma region HUMIDITY_DAMPER
    int damperDirState = eHumidityDamperStatus::DAMPER_OFF;

    if (humiditySensor.value() - humiditySetPoint.value() >= humidityDamperDiffParam.value()) {
      humidityDamperOnOffTimer.setDuration(humidityDamperEnableTimeParam.value(), humidityDamperDisableTimeParam.value() * 1000, false);
      damperDirState = humidityDamperOnOffTimer.isEnabledNow() ? eHumidityDamperStatus::DAMPER_OPEN : eHumidityDamperStatus::DAMPER_OFF;
    } else {
      humidityDamperOnOffTimer.setDuration(35000, 60000);  // 35s ON, 60s OFF
      damperDirState = humidityDamperOnOffTimer.isEnabledNow() ? eHumidityDamperStatus::DAMPER_CLOSE : eHumidityDamperStatus::DAMPER_OFF;
    }

    if (humiditySensor.isOutOfRange()) {
      damperDirState = eHumidityDamperStatus::DAMPER_OFF;
    }

    if ((hasVentilationFail || hasElectricalFail) && failFlagsBlockParam.value()) {
      damperDirState = eHumidityDamperStatus::DAMPER_OPEN;
    }

    digitalWrite(humidityDamperA, damperDirState == eHumidityDamperStatus::DAMPER_OFF ? LOW : damperDirState != eHumidityDamperStatus::DAMPER_OPEN);
    digitalWrite(humidityDamperB, damperDirState == eHumidityDamperStatus::DAMPER_OFF ? LOW : damperDirState == eHumidityDamperStatus::DAMPER_OPEN);
    humidityDamperOutputState.setValueSync(damperDirState, false);

    if (damperDirState != eHumidityDamperStatus::DAMPER_OFF) {
      int timeEnabled = humidityDamperEnableTimeParam.value();
      humidityDamperOutputRamp.start(constrain(timeEnabled, timeEnabled, MIN_ACCELERATION_RAMP_IN_MS));
    } else
      humidityDamperOutputRamp.stop();

#pragma endregion

#pragma region TEMPERATURE_FAN
    if (temperatureFanEnabled.value()) {
      reactiveFanTimer.reset();
    } else if (reactiveFanTimer.waitFor(temperatureFanReactiveParam.value() * 1000 * 60)) {
      temperatureFanEnabled.setValueSync(true);
    }

    bool shouldActivateFan = false;

    if (temperatureFanEnabled.value() && (temperatureSetPoint.value() - temperatureSensor.value()) >= temperatureFanDiffParam.value()) {
      shouldActivateFan = intervalFanStateTimer.waitFor(temperatureFanIntervalParam.value() * 1000);
    } else if (digitalRead(temperatureFanOutput)) {
      intervalFanStateTimer.reset();
    }

    if (((hasVentilationFail || hasElectricalFail) && failFlagsBlockParam.value()) || temperatureSensor.isOutOfRange()) {
      shouldActivateFan = false;
    }

    digitalWrite(temperatureFanOutput, shouldActivateFan);
    temperatureFanOutputState.setValueSync(shouldActivateFan, false);
#pragma endregion

#pragma region TEMPERATURE_INJECTION_MACHINE
    bool shouldActiveInjectionMachine = false;
    int injectionMachineState = eInjectionMachineStatus::MACHINE_OFF;
    injectionMachineOnOffTimer.setDuration(injectionMachineEnableTimeParam.value() * 1000, (injectionMachineDisabledTimeParam.value() + injectionMachineClearTimeParam.value()) * 1000);

    if (shouldActivateFan && injectionMachineEnabled.value() && (temperatureSetPoint.value() - temperatureSensor.value()) >= injectionMachineDiffParam.value()) {
      shouldActiveInjectionMachine = intervalInjectMachineStateTimer.waitFor(injectionMachineIntervalParam.value() * 1000);
    } else if (digitalRead(injectionMachineA)) {
      intervalInjectMachineStateTimer.reset();
    }

    if (!shouldActiveInjectionMachine) {
      injectionMachineOnOffTimer.reset();
      injectionMachineState = eInjectionMachineStatus::MACHINE_OFF;
    } else {
      injectionMachineState = injectionMachineOnOffTimer.isEnabledNow() ? eInjectionMachineStatus::MACHINE_ON : eInjectionMachineStatus::MACHINE_OFF;
      if (injectionMachineState == eInjectionMachineStatus::MACHINE_ON) injectionMachineClearTimer.reset();
    }

    if (injectionMachineState == eInjectionMachineStatus::MACHINE_OFF && !injectionMachineClearTimer.waitFor(injectionMachineClearTimeParam.value() * 1000)) {
      injectionMachineState = eInjectionMachineStatus::MACHINE_CLEAR;
    }

    digitalWrite(injectionMachineA, injectionMachineState != eInjectionMachineStatus::MACHINE_OFF);
    digitalWrite(injectionMachineB, injectionMachineState == eInjectionMachineStatus::MACHINE_ON);
    injectionMachineOutputState.setValueSync(injectionMachineState, false);
#pragma endregion

#pragma region ALARMS
    alarmFlags.VENTILATION_FAILURE = hasVentilationFail;
    alarmFlags.ELECTRICAL_SUPPLY = hasElectricalFail;
    alarmFlags.TEMPERATURE_SENSOR_FAILURE = temperatureSensor.isOutOfRange();
    alarmFlags.HUMIDITY_SENSOR_FAILURE = humiditySensor.isOutOfRange();

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
      reactiveAlarmTimer.reset();
    } else if (reactiveAlarmTimer.waitFor(alarmReactiveParam.value() * 1000 * 60)) {
      alarmEnabled.setValueSync(true);
    }

    if (toggleAlarmTimer.waitFor(900)) {
      toggleAlarmTimer.reset();
      digitalWrite(alarmOutput, shouldActivateAlarm && alarmEnabled.value() ? !digitalRead(alarmOutput) : LOW);
      alarmOutputState.setValueSync(digitalRead(alarmOutput), false);
    }

#pragma endregion

#pragma region DEBUG
    if (debug.waitFor(1000)) {
      debug.reset();
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

      Serial.printf("[OUTPUTS] ");
      if (digitalRead(temperatureFanOutput)) Serial.print("temperatureFanOutput");
      if (digitalRead(injectionMachineA)) Serial.print(" | injectionMachineA");
      if (digitalRead(injectionMachineB)) Serial.print(" | injectionMachineB");
      if (digitalRead(humidityDamperA)) Serial.print(" | humidityDamperA");
      if (digitalRead(humidityDamperB)) Serial.print(" | humidityDamperB");
      if (digitalRead(alarmOutput)) Serial.print(" | alarmOutput");
      Serial.println();

      Serial.printf("[ALARM] enabled: %d, shouldActivateAlarm: %d - ", alarmEnabled.value(), shouldActivateAlarm);
      Serial.println(alarms);
      Serial.printf("[HUMIDITY] damperAction: %s\n", damperDirState == eHumidityDamperStatus::DAMPER_OFF ? "OFF" : damperDirState == eHumidityDamperStatus::DAMPER_OPEN ? "OPEN"
                                                                                                                                                                        : "CLOSE");
      Serial.printf("[FAN] enabled: %d, shouldActivateFan: %d \n", temperatureFanEnabled.value(), shouldActivateFan);
      Serial.printf("[INJECTION] enabled: %d, shouldActiveInjectionMachine: %d , state: %d\n", injectionMachineEnabled.value(), shouldActiveInjectionMachine, injectionMachineState == eInjectionMachineStatus::MACHINE_CLEAR ? "CLEAR" : injectionMachineState == eInjectionMachineStatus::MACHINE_ON ? "ON"
                                                                                                                                                                                                                                                                                                       : "OFF");
      Serial.printf("[FLAG] ventilationFail: %d, electricalFail: %d , \n", hasVentilationFail, hasElectricalFail);

      Serial.println(F("---------------------------------------------------------"));
    }
#pragma endregion
  }
}

#endif