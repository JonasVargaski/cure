#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "global.h"
#include "xTasks/display.h"
#include "xTasks/control.h"
#include "xTasks/main.h"

#include "ADS1X15.h"

#define pinCount 8

void setup()
{
  Serial.begin(115200);

  Wire.begin();

  ads.begin();
  ads.setGain(0);
  ads.setMode(1);

  if (!ads.isConnected())
  {
    Serial.println("ads1115 Connection failed");
  }

  int pins[pinCount] = {13, 12, 14, 27, 26, 25, 33, 32};

  for (int i = 0; i < pinCount; i++)
  {
    pinMode(pins[i], OUTPUT);
  }

  xTaskCreatePinnedToCore(xTaskMain, "mainTask", 10000, NULL, 1, NULL, 0);
  xTaskCreatePinnedToCore(xTaskControl, "controlTask", 10000, NULL, 2, NULL, 0);

  xTaskCreatePinnedToCore(xTaskDisplay, "displayTask", 10000, NULL, 1, NULL, 1);
}

void loop()
{
  sensor_temp.addValue(ads.readADC(0));
  delay(333);
}