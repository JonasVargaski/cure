#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "global.h"
#include "xTasks/control.h"
#include "xTasks/display.h"
#include "xTasks/sensors.h"

void setup()
{
  Serial.begin(115200);

  if (!prefs.begin("global_prefs", false))
  {
    Serial.println("Preferences open failed");
    delay(3000);
    ESP.restart();
  }

  vpMutex = xSemaphoreCreateMutex();

  for (int i = 0; i < pinCount; i++)
  {
    pinMode(pins[i], OUTPUT);
  }

  xTaskCreatePinnedToCore(xTaskControl, "controlTask", 10000, NULL, 2, NULL, 0);

  xTaskCreatePinnedToCore(xTaskDisplay, "displayTask", 10000, NULL, 1, NULL, 1);
  xTaskCreatePinnedToCore(xTaskSensors, "sensorsTask", 10000, NULL, 2, NULL, 1);
}

void loop()
{
}