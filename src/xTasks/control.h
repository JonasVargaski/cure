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
#define SECURITY_MODE_RESET_DIFF 3

enum ePinMap {
  OUT_ALARM = 32,
  OUT_FAN = 13,
  OUT_TEMP_DAMPER_A = 14,
  OUT_TEMP_DAMPER_B = 26,
  OUT_DAMPER_A = 27,
  OUT_DAMPER_B = 25,
  OUT_DAMPER_PWM = 15,
  OUT_INJECTION_A = 12,
  OUT_INJECTION_B = 33,
  IN_ELECTRICAL = 34,
  IN_VENTILATION = 35,
};

// // pin map output
// #define alarmOutput 32
// #define temperatureFanOutput 13
// #define temperatureDamperAOutput 14
// #define temperatureDamperBOutput 26
// #define humidityDamperPwmOutput 15
// #define humidityDamperA 27
// #define humidityDamperB 25
// #define humidityDamperPwm 15
// #define injectionMachineA 12
// #define injectionMachineB 33
// #define electricalInputFlag 34
// #define ventilationInputFlag 35

void resetIOs() {
  pinMode(ePinMap::IN_ELECTRICAL, INPUT_PULLUP);
  pinMode(ePinMap::IN_VENTILATION, INPUT_PULLUP);

  int pins[9] = {ePinMap::OUT_ALARM, ePinMap::OUT_DAMPER_A, ePinMap::OUT_DAMPER_B, ePinMap::OUT_DAMPER_PWM, ePinMap::OUT_FAN, ePinMap::OUT_INJECTION_A, ePinMap::OUT_INJECTION_B, ePinMap::OUT_TEMP_DAMPER_A, ePinMap::OUT_TEMP_DAMPER_B};
  for (int i = 0; i < 8; i++) {
    pinMode(pins[i], OUTPUT);
    digitalWrite(pins[i], LOW);
  }
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

  AccelerationRampPwmModel humidityDamperOutputRamp(ePinMap::OUT_DAMPER_PWM, 0, 1500);
  bool securityModeActivated = false;

  while (!temperatureSensor.complete() || !humiditySensor.complete()) {
    vTaskDelay(pdMS_TO_TICKS(100));
  }

  while (1) {
    vTaskDelay(pdMS_TO_TICKS(15));

#pragma region INPUT_FLAGS
    bool hasVentilationFail = !digitalRead(ePinMap::IN_VENTILATION);
    bool hasElectricalFail = !digitalRead(ePinMap::IN_ELECTRICAL);
#pragma endregion

#pragma region HUMIDITY_DAMPER
    int damperDirState = eHumidityDamperStatus::DAMPER_OFF;

    if (!securityModeActivated) {
      if (humiditySensor.value() - humiditySetPoint.value() >= humidityDamperDiffParam.value()) {
        humidityDamperOnOffTimer.setDuration(humidityDamperEnableTimeParam.value(), humidityDamperDisableTimeParam.value() * 1000, false);
        damperDirState = humidityDamperOnOffTimer.isEnabledNow() ? eHumidityDamperStatus::DAMPER_OPEN : eHumidityDamperStatus::DAMPER_OFF;
      } else {
        humidityDamperOnOffTimer.setDuration(35000, 60000);  // 35s ON, 60s OFF
        damperDirState = humidityDamperOnOffTimer.isEnabledNow() ? eHumidityDamperStatus::DAMPER_CLOSE : eHumidityDamperStatus::DAMPER_OFF;
      }
    }

    if (temperatureSensor.value() - temperatureSetPoint.value() >= securityModeTemperatureDiffParam.value()) {
      damperDirState = eHumidityDamperStatus::DAMPER_OPEN;
      securityModeActivated = true;
    } else if (temperatureSetPoint.value() - temperatureSensor.value() >= securityModeTemperatureDiffParam.value()) {
      damperDirState = eHumidityDamperStatus::DAMPER_CLOSE;
      securityModeActivated = true;
    } else if (abs(temperatureSensor.value() - temperatureSetPoint.value()) <= SECURITY_MODE_RESET_DIFF) {
      securityModeActivated = false;
    }

    if (humiditySensor.isOutOfRange()) {
      damperDirState = eHumidityDamperStatus::DAMPER_OFF;
    }

    if ((hasVentilationFail || hasElectricalFail) && failFlagsBlockParam.value()) {
      damperDirState = eHumidityDamperStatus::DAMPER_OPEN;
    }

    digitalWrite(ePinMap::OUT_DAMPER_A, damperDirState == eHumidityDamperStatus::DAMPER_OFF ? LOW : damperDirState != eHumidityDamperStatus::DAMPER_OPEN);
    digitalWrite(ePinMap::OUT_DAMPER_B, damperDirState == eHumidityDamperStatus::DAMPER_OFF ? LOW : damperDirState == eHumidityDamperStatus::DAMPER_OPEN);
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
    } else if (digitalRead(ePinMap::OUT_FAN)) {
      intervalFanStateTimer.reset();
    }

    if (((hasVentilationFail || hasElectricalFail) && failFlagsBlockParam.value()) || temperatureSensor.isOutOfRange()) {
      shouldActivateFan = false;
    }

    digitalWrite(ePinMap::OUT_FAN, shouldActivateFan);
    temperatureFanOutputState.setValueSync(shouldActivateFan, false);
#pragma endregion

#pragma region TEMPERATURE_INJECTION_MACHINE
    bool shouldActiveInjectionMachine = false;
    int injectionMachineState = eInjectionMachineStatus::MACHINE_OFF;
    injectionMachineOnOffTimer.setDuration(injectionMachineEnableTimeParam.value() * 1000, (injectionMachineDisabledTimeParam.value() + injectionMachineClearTimeParam.value()) * 1000);

    if (shouldActivateFan && injectionMachineEnabled.value() && (temperatureSetPoint.value() - temperatureSensor.value()) >= injectionMachineDiffParam.value()) {
      shouldActiveInjectionMachine = intervalInjectMachineStateTimer.waitFor(injectionMachineIntervalParam.value() * 1000);
    } else if (digitalRead(ePinMap::OUT_INJECTION_A)) {
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

    digitalWrite(ePinMap::OUT_INJECTION_A, injectionMachineState != eInjectionMachineStatus::MACHINE_OFF);
    digitalWrite(ePinMap::OUT_INJECTION_B, injectionMachineState == eInjectionMachineStatus::MACHINE_ON);
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
    } else if (abs(temperatureSensor.value() - temperatureSetPoint.value()) <= SECURITY_MODE_RESET_DIFF) {
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
      digitalWrite(ePinMap::OUT_ALARM, shouldActivateAlarm && alarmEnabled.value() ? !digitalRead(ePinMap::OUT_ALARM) : LOW);
      alarmOutputState.setValueSync(digitalRead(ePinMap::OUT_ALARM), false);
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
      if (digitalRead(ePinMap::OUT_FAN)) Serial.print("OUT_FAN");
      if (digitalRead(ePinMap::OUT_INJECTION_A)) Serial.print(" | OUT_INJECTION_A");
      if (digitalRead(ePinMap::OUT_INJECTION_B)) Serial.print(" | OUT_INJECTION_B");
      if (digitalRead(ePinMap::OUT_DAMPER_A)) Serial.print(" | OUT_DAMPER_A");
      if (digitalRead(ePinMap::OUT_DAMPER_B)) Serial.print(" | OUT_DAMPER_B");
      if (digitalRead(ePinMap::OUT_ALARM)) Serial.print(" | OUT_ALARM");
      Serial.println();

      Serial.printf("[ALARM] enabled: %d, alarmState: %s - ", alarmEnabled.value(), shouldActivateAlarm ? "ON" : "OFF");
      Serial.println(alarms);
      Serial.printf("[HUMIDITY] pwm: %d, damperState: %s\n", ledcRead(0), damperDirState == eHumidityDamperStatus::DAMPER_OFF ? "OFF" : damperDirState == eHumidityDamperStatus::DAMPER_OPEN ? "OPEN"
                                                                                                                                                                                             : "CLOSE");
      Serial.printf("[FAN] enabled: %d, fanState: %s \n", temperatureFanEnabled.value(), shouldActivateFan ? "ON" : "OFF");
      Serial.printf("[INJECTION] enabled: %d, injectionState: %s\n", injectionMachineEnabled.value(), injectionMachineState == eInjectionMachineStatus::MACHINE_CLEAR ? "CLEAR" : injectionMachineState == eInjectionMachineStatus::MACHINE_ON ? "ON"
                                                                                                                                                                                                                                               : "OFF");
      Serial.printf("[FLAG] ventilationFail: %d, electricalFail: %d, securityModeState: %s \n", hasVentilationFail, hasElectricalFail, securityModeActivated ? "ON" : "OFF");

      Serial.println(F("---------------------------------------------------------"));
    }
#pragma endregion
  }
}

#endif