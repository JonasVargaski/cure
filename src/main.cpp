#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "defines/version.h"
#include "global.h"
#include "xTasks/control.h"
#include "xTasks/display.h"
#include "xTasks/sensors.h"
#include "xTasks/wifi.h"

void setup() {
  Serial.begin(115200);

  resetIOs();

  Preferences preferences;
  preferences.begin("VP", true);
  for (Uint16StorageModel* obj : numberDisplayVariables)
    obj->loadValue(&preferences);
  for (BoolStorageModel* obj : booleanDisplayVariables)
    obj->loadValue(&preferences);
  for (TextStorageModel* obj : textDisplayVariables)
    obj->loadValue(&preferences);
  preferences.end();

  firmwareVersion.setValue(FIRMWARE_VERSION);

  variableMutex = xSemaphoreCreateMutex();
  sensorMutex = xSemaphoreCreateMutex();
  outputMutex = xSemaphoreCreateMutex();

  delay(500);
  randomSeed(millis());

  xTaskCreatePinnedToCore(xTaskControl, "controlTask", 4096, NULL, 1, NULL, 0);
  xTaskCreatePinnedToCore(xTaskSensors, "sensorsTask", 4096, NULL, 1, NULL, 1);
  xTaskCreatePinnedToCore(xTaskDisplay, "displayTask", 4096, NULL, 2, NULL, 1);
  xTaskCreatePinnedToCore(xTaskWifi, "wifiTask", 4096, NULL, 3, NULL, 1);
}

void loop() {
}