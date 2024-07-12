#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "defines/version.h"
#include "esp_task_wdt.h"
#include "global.h"
#include "utils/factory_reset.h"
#include "xTasks/control.h"
#include "xTasks/display.h"
#include "xTasks/sensors.h"
#include "xTasks/wifi.h"

AsyncTimerModel workingHoursTimer;

void setup() {
  resetIOs();

  esp_task_wdt_add(NULL);
  esp_task_wdt_add(xTaskControlHandle);
  esp_task_wdt_add(xTaskDisplayHandle);
  esp_task_wdt_add(xTaskSensorsHandle);
  esp_task_wdt_add(xTaskWifiHandle);
  esp_task_wdt_init(5, true);

  variableMutex = xSemaphoreCreateMutex();

  Preferences preferences;
  preferences.begin("vp", true);

  for (DisplayInt *obj : displayIntObjects)
    obj->setValue(preferences.getInt(String(obj->address).c_str(), 0));

  for (DisplayText *obj : displayTextObjects)
    obj->setValue(preferences.getString(String(obj->address).c_str(), "").c_str());

  preferences.end();

  Serial.begin(115200);
  Serial.println(F("\nready!\n"));

  firmwareVersion.setValue(FIRMWARE_VERSION);
  if (!skipFactoryResetFlag.value) factoryReset();

  restartWifiTask();
  xTaskCreatePinnedToCore(xTaskSensors, "sensorsTask", 2048, NULL, 1, &xTaskSensorsHandle, 1);

  delay(3000);
  xTaskCreatePinnedToCore(xTaskControl, "controlTask", 4096, NULL, 1, &xTaskControlHandle, 0);
  xTaskCreatePinnedToCore(xTaskDisplay, "displayTask", 4096, NULL, 2, &xTaskDisplayHandle, 1);
}

void loop() {
  esp_task_wdt_reset();
  if (workingHoursTimer.waitFor(3600000)) workingTimeInHours.setValue(workingTimeInHours.value + 1);
}