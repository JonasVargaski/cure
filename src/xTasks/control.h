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
  AsyncTimerModel checkInputFlagsTimer;
  AsyncTimerModel displayAlarmTimer;

  CyclicTimerModel humidityDamperOnOffTimer;
  CyclicTimerModel injectionMachineOnOffTimer;

  PwmRampModel humidityDamperOutputRamp(ePinMap::OUT_DAMPER_PWM, 0, 2000);

  while (!temperatureSensor.isCompleted() || !humiditySensor.isCompleted()) {
    vTaskDelay(pdMS_TO_TICKS(100));
  }

  while (1) {
    vTaskDelay(pdMS_TO_TICKS(15));

#pragma region SENSORS

    activeAlarms[eDisplayAlarm::TEMPERATURE_SENSOR_FAIL] = temperatureSensor.isOutOfRange();
    activeAlarms[eDisplayAlarm::HUMIDITY_SENSOR_FAIL] = humiditySensor.isOutOfRange();
#pragma endregion

#pragma region INPUT_FLAGS
    if (checkInputFlagsTimer.waitFor(3000)) {
      energySupplyInputState.setValue(!digitalRead(ePinMap::IN_ELECTRICAL), false);
      remoteFailInputState.setValue(!digitalRead(ePinMap::IN_VENTILATION), false);
      checkInputFlagsTimer.reset();
    }

#pragma endregion

#pragma region HUMIDITY_DAMPER
    static bool securityModeActivated = false;
    static int damperDirState = eHumidityDamperStatus::DAMPER_OFF;

    if (temperatureSensor.value - temperatureSetPoint.value >= securityModeTemperatureDiffParam.value) {
      securityModeActivated = true;
      activeAlarms[eDisplayAlarm::SECURITY_MODE_LOW] = false;
      activeAlarms[eDisplayAlarm::SECURITY_MODE_HIGH] = true;
      damperDirState = eHumidityDamperStatus::DAMPER_OPEN;
    } else if (temperatureSetPoint.value - temperatureSensor.value >= securityModeTemperatureDiffParam.value) {
      securityModeActivated = true;
      activeAlarms[eDisplayAlarm::SECURITY_MODE_LOW] = true;
      activeAlarms[eDisplayAlarm::SECURITY_MODE_HIGH] = false;
      damperDirState = eHumidityDamperStatus::DAMPER_CLOSE;
    } else if (abs(temperatureSensor.value - temperatureSetPoint.value) <= securityModeTemperatureDiffParam.value - 4) {
      securityModeActivated = false;
      activeAlarms[eDisplayAlarm::SECURITY_MODE_LOW] = false;
      activeAlarms[eDisplayAlarm::SECURITY_MODE_HIGH] = false;
    }

    if (!securityModeActivated) {
      static bool shouldOpenDamper = false;

      if (humiditySensor.value <= humiditySetPoint.value) {
        shouldOpenDamper = false;
      } else if (shouldOpenDamper || humiditySensor.value - humiditySetPoint.value >= humidityDamperDiffParam.value) {
        humidityDamperOnOffTimer.setDuration(humidityDamperEnableTimeParam.value, humidityDamperDisableTimeParam.value * 1000, true);
        damperDirState = humidityDamperOnOffTimer.isEnabledNow() ? eHumidityDamperStatus::DAMPER_OPEN : eHumidityDamperStatus::DAMPER_OFF;
        shouldOpenDamper = true;
      }

      if (!shouldOpenDamper) {
        humidityDamperOnOffTimer.setDuration(35000, 60000);  // 35s OPEN, 60s CLOSED
        damperDirState = humidityDamperOnOffTimer.isEnabledNow() ? eHumidityDamperStatus::DAMPER_CLOSE : eHumidityDamperStatus::DAMPER_OFF;
      }

      if (humiditySensor.isOutOfRange()) damperDirState = eHumidityDamperStatus::DAMPER_OFF;
    }

    if ((remoteFailInputState.value || energySupplyInputState.value) && failFlagsBlockParam.value) {
      damperDirState = eHumidityDamperStatus::DAMPER_OPEN;
      activeAlarms[eDisplayAlarm::REMOTE_FAIL] = remoteFailInputState.value;
      activeAlarms[eDisplayAlarm::ELECTRICAL_FAIL] = energySupplyInputState.value;
    }

    digitalWrite(ePinMap::OUT_DAMPER_A, damperDirState == eHumidityDamperStatus::DAMPER_OFF ? LOW : damperDirState != eHumidityDamperStatus::DAMPER_OPEN);
    digitalWrite(ePinMap::OUT_DAMPER_B, damperDirState == eHumidityDamperStatus::DAMPER_OFF ? LOW : damperDirState == eHumidityDamperStatus::DAMPER_OPEN);
    humidityDamperOutputState.setValue(damperDirState, false);

    if (damperDirState != eHumidityDamperStatus::DAMPER_OFF) {
      int timeEnabled = humidityDamperEnableTimeParam.value;
      humidityDamperOutputRamp.start(constrain(timeEnabled, timeEnabled, MAX_ACCELERATION_RAMP_TIME_MS));
    } else {
      humidityDamperOutputRamp.stop();
    }

    if (humiditySensor.value - humiditySetPoint.value > ARROW_DIFF) {
      humidityStatusFlagIndicator.setValue(eArrowIcon::ARROW_UP, false);
    } else if (humiditySetPoint.value - humiditySensor.value > ARROW_DIFF) {
      humidityStatusFlagIndicator.setValue(eArrowIcon::ARROW_DOWN, false);
    } else {
      humidityStatusFlagIndicator.setValue(eArrowIcon::ARROW_NONE, false);
    }

    if (humiditySensor.value - humiditySetPoint.value >= alarmHumidityDiffParam.value) {
      activeAlarms[eDisplayAlarm::HUMIDITY_LOW] = false;
      activeAlarms[eDisplayAlarm::HUMIDITY_HIGH] = true;
    } else if (humiditySetPoint.value - humiditySensor.value >= alarmHumidityDiffParam.value) {
      activeAlarms[eDisplayAlarm::HUMIDITY_LOW] = true;
      activeAlarms[eDisplayAlarm::HUMIDITY_HIGH] = false;
    } else if (abs(humiditySensor.value - humiditySetPoint.value) < alarmHumidityDiffParam.value - 1) {
      activeAlarms[eDisplayAlarm::HUMIDITY_LOW] = false;
      activeAlarms[eDisplayAlarm::HUMIDITY_HIGH] = false;
    }
#pragma endregion

#pragma region TEMPERATURE_FAN
    static bool shouldActivateFan = false;

    if (temperatureFanEnabled.value) {
      reactiveFanTimer.reset();

      if (!shouldActivateFan && (temperatureSetPoint.value - temperatureSensor.value) >= temperatureFanDiffParam.value) {
        shouldActivateFan = intervalFanStateTimer.waitFor(5000);
      } else if (temperatureSensor.value >= temperatureSetPoint.value) {
        shouldActivateFan = false;
      }

    } else {
      shouldActivateFan = false;
      if (temperatureFanReactiveParam.value > 0 && reactiveFanTimer.waitFor(temperatureFanReactiveParam.value * 1000 * 60)) {
        temperatureFanEnabled.setValue(true);
      }
    }

    if (shouldActivateFan) {
      intervalFanStateTimer.reset();

      if (temperatureSensor.isOutOfRange() || (failFlagsBlockParam.value && (remoteFailInputState.value || energySupplyInputState.value))) {
        shouldActivateFan = false;
      }
    }

    digitalWrite(ePinMap::OUT_FAN, shouldActivateFan);
    temperatureFanOutputState.setValue(shouldActivateFan, false);

    if (temperatureSensor.value - temperatureSetPoint.value > ARROW_DIFF) {
      tempStatusFlagIndicator.setValue(eArrowIcon::ARROW_UP, false);
    } else if (temperatureSetPoint.value - temperatureSensor.value > ARROW_DIFF) {
      tempStatusFlagIndicator.setValue(eArrowIcon::ARROW_DOWN, false);
    } else {
      tempStatusFlagIndicator.setValue(eArrowIcon::ARROW_NONE, false);
    }

    if (temperatureSensor.value - temperatureSetPoint.value >= alarmTemperatureDiffParam.value) {
      activeAlarms[eDisplayAlarm::TEMPERATURE_LOW] = false;
      activeAlarms[eDisplayAlarm::TEMPERATURE_HIGH] = true;
    } else if (temperatureSetPoint.value - temperatureSensor.value >= alarmTemperatureDiffParam.value) {
      activeAlarms[eDisplayAlarm::TEMPERATURE_LOW] = true;
      activeAlarms[eDisplayAlarm::TEMPERATURE_HIGH] = false;
    } else if (abs(temperatureSensor.value - temperatureSetPoint.value) < alarmTemperatureDiffParam.value - 1) {
      activeAlarms[eDisplayAlarm::TEMPERATURE_LOW] = false;
      activeAlarms[eDisplayAlarm::TEMPERATURE_HIGH] = false;
    }
#pragma endregion

#pragma region TEMPERATURE_INJECTION_MACHINE
    static int injectionMachineState = eInjectionMachineStatus::MACHINE_OFF;
    bool canActivateMachine = shouldActivateFan && injectionMachineEnabled.value;

    injectionMachineOnOffTimer.setDuration(injectionMachineEnableTimeParam.value * 1000, (injectionMachineDisabledTimeParam.value + injectionMachineClearTimeParam.value) * 1000);

    if (canActivateMachine && (temperatureSetPoint.value - temperatureSensor.value) >= injectionMachineDiffParam.value) {
      if (injectionMachineState == eInjectionMachineStatus::MACHINE_OFF && injectionMachineOnOffTimer.isEnabledNow()) {
        injectionMachineState = eInjectionMachineStatus::MACHINE_ON;
      }
    } else if (injectionMachineState == eInjectionMachineStatus::MACHINE_OFF) {
      injectionMachineOnOffTimer.reset();
    }

    if (injectionMachineState == eInjectionMachineStatus::MACHINE_CLEAR && injectionMachineClearTimer.waitFor(injectionMachineClearTimeParam.value * 1000)) {
      injectionMachineState = eInjectionMachineStatus::MACHINE_OFF;
    } else if (injectionMachineState == eInjectionMachineStatus::MACHINE_ON && (!injectionMachineOnOffTimer.isEnabledNow() || !canActivateMachine)) {
      injectionMachineClearTimer.reset();
      injectionMachineState = eInjectionMachineStatus::MACHINE_CLEAR;
    }

    digitalWrite(ePinMap::OUT_INJECTION_A, injectionMachineState != eInjectionMachineStatus::MACHINE_OFF);
    digitalWrite(ePinMap::OUT_INJECTION_B, injectionMachineState == eInjectionMachineStatus::MACHINE_ON);
    injectionMachineOutputStateA.setValue(digitalRead(ePinMap::OUT_INJECTION_A), false);
    injectionMachineOutputStateB.setValue(digitalRead(ePinMap::OUT_INJECTION_B), false);
#pragma endregion

#pragma region ALARMS
    bool shouldActivateAlarm = false;

    if (activeAlarms[eDisplayAlarm::REMOTE_FAIL] && alarmVentilationTypeParam.value) {
      shouldActivateAlarm = true;
    } else if (activeAlarms[eDisplayAlarm::ELECTRICAL_FAIL] && alarmVentilationTypeParam.value) {
      shouldActivateAlarm = true;
    } else if (activeAlarms[eDisplayAlarm::TEMPERATURE_HIGH] && (alarmTemperatureTypeParam.value == eAlarmEnableType::HIGH_VALUE || alarmTemperatureTypeParam.value == eAlarmEnableType::ALL)) {
      shouldActivateAlarm = true;
    } else if (activeAlarms[eDisplayAlarm::TEMPERATURE_LOW] && (alarmTemperatureTypeParam.value == eAlarmEnableType::LOW_VALUE || alarmTemperatureTypeParam.value == eAlarmEnableType::ALL)) {
      shouldActivateAlarm = true;
    } else if (activeAlarms[eDisplayAlarm::HUMIDITY_HIGH] && (alarmHumidityTypeParam.value == eAlarmEnableType::HIGH_VALUE || alarmHumidityTypeParam.value == eAlarmEnableType::ALL)) {
      shouldActivateAlarm = true;
    } else if (activeAlarms[eDisplayAlarm::HUMIDITY_LOW] && (alarmHumidityTypeParam.value == eAlarmEnableType::LOW_VALUE || alarmHumidityTypeParam.value == eAlarmEnableType::ALL)) {
      shouldActivateAlarm = true;
    } else if (activeAlarms[eDisplayAlarm::SECURITY_MODE_HIGH] && (alarmSecurityTypeParam.value == eAlarmEnableType::HIGH_VALUE || alarmSecurityTypeParam.value == eAlarmEnableType::ALL)) {
      shouldActivateAlarm = true;
    } else if (activeAlarms[eDisplayAlarm::SECURITY_MODE_LOW] && (alarmSecurityTypeParam.value == eAlarmEnableType::LOW_VALUE || alarmSecurityTypeParam.value == eAlarmEnableType::ALL)) {
      shouldActivateAlarm = true;
    }

    static int alarmDisplayIndex;

    if (displayAlarmTimer.waitFor(3000)) {
      int oldAlarmIndex = alarmDisplayIndex;

      do {
        alarmDisplayIndex = (alarmDisplayIndex + 1) % eDisplayAlarm::DISPLAY_ALARM_MAX_SIZE;

        if (activeAlarms[alarmDisplayIndex]) {
          alarmReasons.setValue(alarmDisplayIndex, false);
          break;
        }

      } while (alarmDisplayIndex != oldAlarmIndex);

      if (!activeAlarms[alarmDisplayIndex]) alarmReasons.setValue(eDisplayAlarm::ALARM_NONE, false);
      displayAlarmTimer.reset();
    }

    if (alarmEnabled.value) {
      reactiveAlarmTimer.reset();
    } else if (alarmReactiveParam.value > 0 && reactiveAlarmTimer.waitFor(alarmReactiveParam.value * 1000 * 60)) {
      alarmEnabled.setValue(true);
    }

    if (toggleAlarmTimer.waitFor(850)) {
      toggleAlarmTimer.reset();
      digitalWrite(ePinMap::OUT_ALARM, shouldActivateAlarm && alarmEnabled.value ? !digitalRead(ePinMap::OUT_ALARM) : LOW);
      alarmOutputState.setValue(digitalRead(ePinMap::OUT_ALARM), false);
    }

#pragma endregion
  }
}

#endif