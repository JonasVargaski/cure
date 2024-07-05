#ifndef _TASK_CONTROL_
#define _TASK_CONTROL_

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>

#include "enums.h"
#include "global.h"
#include "model/async_timer_model.h"
#include "model/cyclic_timer_model.h"
#include "model/pwm_ramp_model.h"
#include "utils/memory.h"

#define MAX_ACCELERATION_RAMP_TIME_MS 1000
#define ARROW_DIFF 3

enum ePinMap {
  IN_ELECTRICAL = 15,
  IN_VENTILATION = 2,
  OUT_ALARM = 13,
  OUT_FAN = 26,
  OUT_DAMPER_A = 14,
  OUT_DAMPER_B = 33,
  OUT_DAMPER_PWM = 32,
  OUT_INJECTION_A = 25,
  OUT_INJECTION_B = 27,
};

void resetIOs() {
  pinMode(ePinMap::IN_ELECTRICAL, INPUT_PULLDOWN);
  pinMode(ePinMap::IN_VENTILATION, INPUT_PULLDOWN);

  int pins[6] = {ePinMap::OUT_ALARM, ePinMap::OUT_DAMPER_A, ePinMap::OUT_DAMPER_B, ePinMap::OUT_FAN, ePinMap::OUT_INJECTION_A, ePinMap::OUT_INJECTION_B};
  for (int i = 0; i < 6; i++) {
    pinMode(pins[i], OUTPUT);
    digitalWrite(pins[i], LOW);
  }
}

TaskHandle_t xTaskControlHandle;

void xTaskControl(void *parameter) {
  AsyncTimerModel toggleAlarmTimer;
  AsyncTimerModel reactiveAlarmTimer;
  AsyncTimerModel reactiveFanTimer;
  AsyncTimerModel intervalFanStateTimer;
  AsyncTimerModel injectionMachineClearTimer(true);
  AsyncTimerModel checkVentilationTimer;
  AsyncTimerModel checkElectricalTimer;
  AsyncTimerModel displayAlarmTimer;

  CyclicTimerModel humidityDamperOnOffTimer;
  CyclicTimerModel injectionMachineOnOffTimer;

  PwmRampModel humidityDamperOutputRamp(ePinMap::OUT_DAMPER_PWM, 0, 2000);

  while (!temperatureSensor.complete() || !humiditySensor.complete()) {
    vTaskDelay(pdMS_TO_TICKS(100));
  }

  while (1) {
    vTaskDelay(pdMS_TO_TICKS(15));

#pragma region SENSORS
    int tempSensorValue = temperatureSensor.value();
    int humidSensorValue = humiditySensor.value();

    activeAlarms[eDisplayAlarm::TEMPERATURE_SENSOR_FAIL] = temperatureSensor.isOutOfRange();
    activeAlarms[eDisplayAlarm::HUMIDITY_SENSOR_FAIL] = humiditySensor.isOutOfRange();
#pragma endregion

#pragma region INPUT_FLAGS
    bool hasRemoteFail = !digitalRead(ePinMap::IN_VENTILATION) ? checkVentilationTimer.waitFor(3000) : checkVentilationTimer.reset();
    bool hasEnergySupplyFail = !digitalRead(ePinMap::IN_ELECTRICAL) ? checkElectricalTimer.waitFor(3000) : checkElectricalTimer.reset();

    energySupplyInputState.setValueSync(hasEnergySupplyFail, false);
    remoteFailInputState.setValueSync(hasRemoteFail, false);

#pragma endregion

#pragma region HUMIDITY_DAMPER
    bool securityModeActivated = false;
    int damperDirState = eHumidityDamperStatus::DAMPER_OFF;

    if (tempSensorValue - temperatureSetPoint.value() >= securityModeTemperatureDiffParam.value()) {
      securityModeActivated = true;
      activeAlarms[eDisplayAlarm::SECURITY_MODE_LOW] = false;
      activeAlarms[eDisplayAlarm::SECURITY_MODE_HIGH] = true;
      damperDirState = eHumidityDamperStatus::DAMPER_OPEN;
    } else if (temperatureSetPoint.value() - tempSensorValue >= securityModeTemperatureDiffParam.value()) {
      securityModeActivated = true;
      activeAlarms[eDisplayAlarm::SECURITY_MODE_LOW] = true;
      activeAlarms[eDisplayAlarm::SECURITY_MODE_HIGH] = false;
      damperDirState = eHumidityDamperStatus::DAMPER_CLOSE;
    } else if (abs(tempSensorValue - temperatureSetPoint.value()) <= 5) {
      securityModeActivated = false;
      activeAlarms[eDisplayAlarm::SECURITY_MODE_LOW] = false;
      activeAlarms[eDisplayAlarm::SECURITY_MODE_HIGH] = false;
    }

    if (!securityModeActivated) {
      static bool shouldOpenDamper = false;

      if (humidSensorValue <= humiditySetPoint.value()) {
        shouldOpenDamper = false;
      } else if (shouldOpenDamper || humidSensorValue - humiditySetPoint.value() >= humidityDamperDiffParam.value()) {
        humidityDamperOnOffTimer.setDuration(humidityDamperEnableTimeParam.value(), humidityDamperDisableTimeParam.value() * 1000, false);
        damperDirState = humidityDamperOnOffTimer.isEnabledNow() ? eHumidityDamperStatus::DAMPER_OPEN : eHumidityDamperStatus::DAMPER_OFF;
        shouldOpenDamper = true;
      }

      if (!shouldOpenDamper) {
        humidityDamperOnOffTimer.setDuration(35000, 60000);  // 35s OPEN, 60s CLOSED
        damperDirState = humidityDamperOnOffTimer.isEnabledNow() ? eHumidityDamperStatus::DAMPER_CLOSE : eHumidityDamperStatus::DAMPER_OFF;
      }

      if (humiditySensor.isOutOfRange()) damperDirState = eHumidityDamperStatus::DAMPER_OFF;
    }

    if ((hasRemoteFail || hasEnergySupplyFail) && failFlagsBlockParam.value()) {
      damperDirState = eHumidityDamperStatus::DAMPER_OPEN;
      activeAlarms[eDisplayAlarm::REMOTE_FAIL] = hasRemoteFail;
      activeAlarms[eDisplayAlarm::ELECTRICAL_FAIL] = hasEnergySupplyFail;
    }

    digitalWrite(ePinMap::OUT_DAMPER_A, damperDirState == eHumidityDamperStatus::DAMPER_OFF ? LOW : damperDirState != eHumidityDamperStatus::DAMPER_OPEN);
    digitalWrite(ePinMap::OUT_DAMPER_B, damperDirState == eHumidityDamperStatus::DAMPER_OFF ? LOW : damperDirState == eHumidityDamperStatus::DAMPER_OPEN);
    humidityDamperOutputState.setValueSync(damperDirState, false);

    if (damperDirState != eHumidityDamperStatus::DAMPER_OFF) {
      int timeEnabled = humidityDamperEnableTimeParam.value();
      humidityDamperOutputRamp.start(constrain(timeEnabled, timeEnabled, MAX_ACCELERATION_RAMP_TIME_MS));
    } else {
      humidityDamperOutputRamp.stop();
    }

    if (humidSensorValue - humiditySetPoint.value() > ARROW_DIFF) {
      humidityStatusFlagIndicator.setValueSync(eArrowIcon::ARROW_UP, false);
    } else if (humiditySetPoint.value() - humidSensorValue > ARROW_DIFF) {
      humidityStatusFlagIndicator.setValueSync(eArrowIcon::ARROW_DOWN, false);
    } else {
      humidityStatusFlagIndicator.setValueSync(eArrowIcon::ARROW_NONE, false);
    }

    if (humidSensorValue - humiditySetPoint.value() >= alarmHumidityDiffParam.value()) {
      activeAlarms[eDisplayAlarm::HUMIDITY_LOW] = false;
      activeAlarms[eDisplayAlarm::HUMIDITY_HIGH] = true;
    } else if (humiditySetPoint.value() - humidSensorValue >= alarmHumidityDiffParam.value()) {
      activeAlarms[eDisplayAlarm::HUMIDITY_LOW] = true;
      activeAlarms[eDisplayAlarm::HUMIDITY_HIGH] = false;
    } else if (abs(humidSensorValue - humiditySetPoint.value()) < alarmHumidityDiffParam.value() - 1) {
      activeAlarms[eDisplayAlarm::HUMIDITY_LOW] = false;
      activeAlarms[eDisplayAlarm::HUMIDITY_HIGH] = false;
    }
#pragma endregion

#pragma region TEMPERATURE_FAN
    static bool shouldActivateFan = false;

    if (temperatureFanEnabled.value()) {
      reactiveFanTimer.reset();

      if (!shouldActivateFan && (temperatureSetPoint.value() - tempSensorValue) >= temperatureFanDiffParam.value()) {
        shouldActivateFan = intervalFanStateTimer.waitFor(5000);
      } else if (tempSensorValue >= temperatureSetPoint.value()) {
        shouldActivateFan = false;
      }

    } else {
      shouldActivateFan = false;
      if (temperatureFanReactiveParam.value() > 0 && reactiveFanTimer.waitFor(temperatureFanReactiveParam.value() * 1000 * 60)) {
        temperatureFanEnabled.setValueSync(true);
      }
    }

    if (shouldActivateFan) {
      intervalFanStateTimer.reset();

      if (temperatureSensor.isOutOfRange() || (failFlagsBlockParam.value() && (hasRemoteFail || hasEnergySupplyFail))) {
        shouldActivateFan = false;
      }
    }

    digitalWrite(ePinMap::OUT_FAN, shouldActivateFan);
    temperatureFanOutputState.setValueSync(shouldActivateFan, false);

    if (tempSensorValue - temperatureSetPoint.value() > ARROW_DIFF) {
      tempStatusFlagIndicator.setValueSync(eArrowIcon::ARROW_UP, false);
    } else if (temperatureSetPoint.value() - tempSensorValue > ARROW_DIFF) {
      tempStatusFlagIndicator.setValueSync(eArrowIcon::ARROW_DOWN, false);
    } else {
      tempStatusFlagIndicator.setValueSync(eArrowIcon::ARROW_NONE, false);
    }

    if (tempSensorValue - temperatureSetPoint.value() >= alarmTemperatureDiffParam.value()) {
      activeAlarms[eDisplayAlarm::TEMPERATURE_LOW] = false;
      activeAlarms[eDisplayAlarm::TEMPERATURE_HIGH] = true;
    } else if (temperatureSetPoint.value() - tempSensorValue >= alarmTemperatureDiffParam.value()) {
      activeAlarms[eDisplayAlarm::TEMPERATURE_LOW] = true;
      activeAlarms[eDisplayAlarm::TEMPERATURE_HIGH] = false;
    } else if (abs(tempSensorValue - temperatureSetPoint.value()) < alarmTemperatureDiffParam.value() - 1) {
      activeAlarms[eDisplayAlarm::TEMPERATURE_LOW] = false;
      activeAlarms[eDisplayAlarm::TEMPERATURE_HIGH] = false;
    }
#pragma endregion

#pragma region TEMPERATURE_INJECTION_MACHINE
    static int injectionMachineState = eInjectionMachineStatus::MACHINE_OFF;
    bool canActivateMachine = shouldActivateFan && injectionMachineEnabled.value();

    injectionMachineOnOffTimer.setDuration(injectionMachineEnableTimeParam.value() * 1000, (injectionMachineDisabledTimeParam.value() + injectionMachineClearTimeParam.value()) * 1000);

    if (canActivateMachine && (temperatureSetPoint.value() - tempSensorValue) >= injectionMachineDiffParam.value()) {
      if (injectionMachineState == eInjectionMachineStatus::MACHINE_OFF && injectionMachineOnOffTimer.isEnabledNow()) {
        injectionMachineState = eInjectionMachineStatus::MACHINE_ON;
      }
    } else if (injectionMachineState == eInjectionMachineStatus::MACHINE_OFF) {
      injectionMachineOnOffTimer.reset();
    }

    if (injectionMachineState == eInjectionMachineStatus::MACHINE_CLEAR && injectionMachineClearTimer.waitFor(injectionMachineClearTimeParam.value() * 1000)) {
      injectionMachineState = eInjectionMachineStatus::MACHINE_OFF;
    } else if (injectionMachineState == eInjectionMachineStatus::MACHINE_ON && (!injectionMachineOnOffTimer.isEnabledNow() || !canActivateMachine)) {
      injectionMachineClearTimer.reset();
      injectionMachineState = eInjectionMachineStatus::MACHINE_CLEAR;
    }

    digitalWrite(ePinMap::OUT_INJECTION_A, injectionMachineState != eInjectionMachineStatus::MACHINE_OFF);
    digitalWrite(ePinMap::OUT_INJECTION_B, injectionMachineState == eInjectionMachineStatus::MACHINE_ON);
    injectionMachineOutputStateA.setValueSync(digitalRead(ePinMap::OUT_INJECTION_A), false);
    injectionMachineOutputStateB.setValueSync(digitalRead(ePinMap::OUT_INJECTION_B), false);
#pragma endregion

#pragma region ALARMS
    bool shouldActivateAlarm = false;

    if (activeAlarms[eDisplayAlarm::REMOTE_FAIL] && alarmVentilationTypeParam.value()) {
      shouldActivateAlarm = true;
    } else if (activeAlarms[eDisplayAlarm::ELECTRICAL_FAIL] && alarmVentilationTypeParam.value()) {
      shouldActivateAlarm = true;
    } else if (activeAlarms[eDisplayAlarm::TEMPERATURE_HIGH] && (alarmTemperatureTypeParam.value() == eAlarmEnableType::HIGH_VALUE || alarmTemperatureTypeParam.value() == eAlarmEnableType::ALL)) {
      shouldActivateAlarm = true;
    } else if (activeAlarms[eDisplayAlarm::TEMPERATURE_LOW] && (alarmTemperatureTypeParam.value() == eAlarmEnableType::LOW_VALUE || alarmTemperatureTypeParam.value() == eAlarmEnableType::ALL)) {
      shouldActivateAlarm = true;
    } else if (activeAlarms[eDisplayAlarm::HUMIDITY_HIGH] && (alarmHumidityTypeParam.value() == eAlarmEnableType::HIGH_VALUE || alarmHumidityTypeParam.value() == eAlarmEnableType::ALL)) {
      shouldActivateAlarm = true;
    } else if (activeAlarms[eDisplayAlarm::HUMIDITY_LOW] && (alarmHumidityTypeParam.value() == eAlarmEnableType::LOW_VALUE || alarmHumidityTypeParam.value() == eAlarmEnableType::ALL)) {
      shouldActivateAlarm = true;
    } else if (activeAlarms[eDisplayAlarm::SECURITY_MODE_HIGH] && (alarmSecurityTypeParam.value() == eAlarmEnableType::HIGH_VALUE || alarmSecurityTypeParam.value() == eAlarmEnableType::ALL)) {
      shouldActivateAlarm = true;
    } else if (activeAlarms[eDisplayAlarm::SECURITY_MODE_LOW] && (alarmSecurityTypeParam.value() == eAlarmEnableType::LOW_VALUE || alarmSecurityTypeParam.value() == eAlarmEnableType::ALL)) {
      shouldActivateAlarm = true;
    }

    static int alarmDisplayIndex;

    if (displayAlarmTimer.waitFor(3000)) {
      int oldAlarmIndex = alarmDisplayIndex;

      do {
        alarmDisplayIndex = (alarmDisplayIndex + 1) % eDisplayAlarm::DISPLAY_ALARM_MAX_SIZE;

        if (activeAlarms[alarmDisplayIndex]) {
          alarmReasons.setValueSync(alarmDisplayIndex, false);
          break;
        }

      } while (alarmDisplayIndex != oldAlarmIndex);

      if (!activeAlarms[alarmDisplayIndex]) alarmReasons.setValueSync(eDisplayAlarm::ALARM_NONE, false);
      displayAlarmTimer.reset();
    }

    if (alarmEnabled.value()) {
      reactiveAlarmTimer.reset();
    } else if (alarmReactiveParam.value() > 0 && reactiveAlarmTimer.waitFor(alarmReactiveParam.value() * 1000 * 60)) {
      alarmEnabled.setValueSync(true);
    }

    if (toggleAlarmTimer.waitFor(850)) {
      toggleAlarmTimer.reset();
      digitalWrite(ePinMap::OUT_ALARM, shouldActivateAlarm && alarmEnabled.value() ? !digitalRead(ePinMap::OUT_ALARM) : LOW);
      alarmOutputState.setValueSync(digitalRead(ePinMap::OUT_ALARM), false);
    }

#pragma endregion
  }
}

#endif