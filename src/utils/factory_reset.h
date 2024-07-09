#ifndef _FACTORY_RESET_UTILS_
#define _FACTORY_RESET_UTILS_

#include <Arduino.h>

#include "global.h"

void factoryReset() {
  temperatureSetPoint.setValueSync(80);
  humiditySetPoint.setValueSync(80);
  alarmReactiveParam.setValueSync(20);
  alarmHumidityDiffParam.setValueSync(8);
  alarmTemperatureDiffParam.setValueSync(8);
  alarmTemperatureTypeParam.setValueSync(3);
  alarmHumidityTypeParam.setValueSync(0);
  alarmSecurityTypeParam.setValueSync(0);
  securityModeTemperatureDiffParam.setValueSync(100);
  remotePasswordParam.setValueSync(0);
  temperatureFanDiffParam.setValueSync(1);
  humidityDamperDiffParam.setValueSync(1);
  humidityDamperEnableTimeParam.setValueSync(500);
  humidityDamperDisableTimeParam.setValueSync(30);
  injectionMachineDiffParam.setValueSync(2);
  injectionMachineClearTimeParam.setValueSync(10);
  injectionMachineEnableTimeParam.setValueSync(20);
  injectionMachineDisabledTimeParam.setValueSync(60);
  temperatureFanReactiveParam.setValueSync(15);
  alarmEnabled.setValueSync(0);
  temperatureFanEnabled.setValueSync(0);
  injectionMachineEnabled.setValueSync(0);
  alarmVentilationTypeParam.setValueSync(1);
  alarmElectricalSupplyTypeParam.setValueSync(1);
  failFlagsBlockParam.setValueSync(1);
  humiditySensorType.setValueSync(false);
  temperatureSensorType.setValueSync(false);
  wifiSsidParam.setValueSync("");
  wifiPasswordParam.setValueSync("");
  firmwareVersion.setValueSync("");
  skipFactoryResetFlag.setValueSync(true);

  Serial.println(F("\n[FACTORY RESET] done!\n"));
  ESP.restart();
}

#endif