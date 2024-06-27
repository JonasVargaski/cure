#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "defines/version.h"
#include "esp_task_wdt.h"
#include "global.h"
#include "xTasks/control.h"
#include "xTasks/display.h"
#include "xTasks/sensors.h"
#include "xTasks/wifi.h"

void setup() {
  esp_task_wdt_add(NULL);
  esp_task_wdt_add(xTaskControlHandle);
  esp_task_wdt_add(xTaskDisplayHandle);
  esp_task_wdt_add(xTaskSensorsHandle);
  esp_task_wdt_add(xTaskWifiHandle);

  esp_task_wdt_init(5, true);

  Serial.begin(115200);
  Serial.println(F("\nready!\n\n"));

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

  xTaskCreatePinnedToCore(xTaskControl, "controlTask", 4096, NULL, 1, &xTaskControlHandle, 0);
  xTaskCreatePinnedToCore(xTaskSensors, "sensorsTask", 2048, NULL, 1, &xTaskSensorsHandle, 1);
  xTaskCreatePinnedToCore(xTaskDisplay, "displayTask", 4096, NULL, 2, &xTaskDisplayHandle, 1);
  xTaskCreatePinnedToCore(xTaskWifi, "wifiTask", 8048, NULL, 3, &xTaskWifiHandle, 1);
}

void loop() {
  esp_task_wdt_reset();
}