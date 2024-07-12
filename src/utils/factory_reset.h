#ifndef _FACTORY_RESET_UTILS_
#define _FACTORY_RESET_UTILS_

#include <Arduino.h>

#include "global.h"

void factoryReset() {
  temperatureSetPoint.setValue(80);
  humiditySetPoint.setValue(80);
  alarmReactiveParam.setValue(20);
  alarmHumidityDiffParam.setValue(8);
  alarmTemperatureDiffParam.setValue(8);
  alarmTemperatureTypeParam.setValue(3);
  alarmHumidityTypeParam.setValue(0);
  alarmSecurityTypeParam.setValue(0);
  securityModeTemperatureDiffParam.setValue(99);
  remotePasswordParam.setValue(0);
  temperatureFanDiffParam.setValue(1);
  humidityDamperDiffParam.setValue(1);
  humidityDamperEnableTimeParam.setValue(500);
  humidityDamperDisableTimeParam.setValue(30);
  injectionMachineDiffParam.setValue(2);
  injectionMachineClearTimeParam.setValue(10);
  injectionMachineEnableTimeParam.setValue(20);
  injectionMachineDisabledTimeParam.setValue(60);
  temperatureFanReactiveParam.setValue(15);
  alarmEnabled.setValue(0);
  temperatureFanEnabled.setValue(0);
  injectionMachineEnabled.setValue(0);
  alarmVentilationTypeParam.setValue(1);
  alarmElectricalSupplyTypeParam.setValue(1);
  failFlagsBlockParam.setValue(0);
  humiditySensorType.setValue(false);
  temperatureSensorType.setValue(false);
  wifiSsidParam.setValue("");
  wifiPasswordParam.setValue("");
  firmwareVersion.setValue("");
  skipFactoryResetFlag.setValue(true);

  Serial.println(F("\n[FACTORY RESET] done!\n"));
  ESP.restart();
}

#endif