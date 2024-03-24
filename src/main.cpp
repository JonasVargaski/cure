#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "global.h"
#include "xTasks/control.h"
#include "xTasks/display.h"
#include "xTasks/sensors.h"

void setup() {
  Serial.begin(115200);

  for (int i = 0; i < pinCount; i++) {
    pinMode(pins[i], OUTPUT);
    digitalWrite(pins[i], LOW);
  }

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