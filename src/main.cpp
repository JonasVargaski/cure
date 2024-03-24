#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "global.h"
#include "xTasks/control.h"
#include "xTasks/display.h"
#include "xTasks/sensors.h"

#define alarmOutput 32
#define temperatureFanOutput 13
#define temperatureDamperAOutput 14
#define temperatureDamperBOutput 26
#define humidityDamperPwmOutput 15
#define humidityDamperA 27
#define humidityDamperB 25
#define injectionScrewA 12
#define injectionScrewB 33

void setup() {
  Serial.begin(115200);

  configureOutputs();

  Preferences preferences;
  preferences.begin("VP", true);

  temperatureSetPoint.loadValue(&preferences);
  humiditySetPoint.loadValue(&preferences);
  temperatureFanEnabled.loadValue(&preferences);
  alarmEnabled.loadValue(&preferences);
  injectionScrewEnabled.loadValue(&preferences);
  wifiSsidParam.loadValue(&preferences);
  wifiPasswordParam.loadValue(&preferences);

  preferences.end();

  variableMutex = xSemaphoreCreateMutex();
  sensorMutex = xSemaphoreCreateMutex();

  delay(500);

  xTaskCreatePinnedToCore(xTaskControl, "controlTask", 4096, NULL, 1, NULL, 0);
  xTaskCreatePinnedToCore(xTaskSensors, "sensorsTask", 4096, NULL, 1, NULL, 1);
  xTaskCreatePinnedToCore(xTaskDisplay, "displayTask", 4096, NULL, 2, NULL, 1);
}

void loop() {
}