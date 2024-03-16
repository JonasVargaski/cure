#ifndef _TASK_DISPLAY_
#define _TASK_DISPLAY_

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>

#include "global.h"
#include <DWIN.h>

void onHMIEvent(String address, int wordValue, String message, String response)
{
  // Serial.println("OnEvent : [ A : " + address + " | D : " + String(lastByte, HEX) + " | M : " + message + " | R : " + response + " ]");
  Serial.print("[DEBUG display] vp = ");
  Serial.print(address);
  Serial.print(" | ");
  Serial.println(wordValue);

  if (address == "1001")
  {
    temperatureSetPoint.setValue(wordValue);
  }
  else if (address == "1007")
  {
    humiditySetPoint.setValue(wordValue);
  }
  else if (address == "1003")
  {
    fanSetPoint.setValue(wordValue);
  }
  else if (address == "1004")
  {
    buzzerSetPoint.setValue(wordValue);
  }
  else if (address == "1005")
  {
    motorSetPoint.setValue(wordValue);
  }
}

void xTaskDisplay(void *parameter)
{
  DWIN hmi(Serial2, 16, 17, 115200);
  hmi.hmiCallBack(onHMIEvent);

  while (1)
  {
    hmi.setVPWord(temperatureSensor.getVp(), temperatureSensor.getValue());
    hmi.setVPWord(humiditySensor.getVp(), humiditySensor.getValue());

    hmi.setVPWord(temperatureSetPoint.getVp(), temperatureSetPoint.getValue());
    hmi.setVPWord(humiditySetPoint.getVp(), humiditySetPoint.getValue());

    hmi.setVPWord(fanSetPoint.getVp(), fanSetPoint.getValue());
    hmi.setVPWord(buzzerSetPoint.getVp(), buzzerSetPoint.getValue());
    hmi.setVPWord(motorSetPoint.getVp(), motorSetPoint.getValue());

    hmi.handle();
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

#endif