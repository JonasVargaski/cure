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
}

void xTaskDisplay(void *parameter)
{
  DWIN hmi(Serial2, 16, 17, 115200);
  hmi.hmiCallBack(onHMIEvent);

  hmi.setVP(0x1001, 104);
  hmi.setVP(0x1003, 1);

  while (1)
  {
    float voltage = sensor_temp.value() * 0.1875 / 1000;
    float temperature = voltage * 100.0;

    hmi.setVPWord(0x1002, temperature);

    hmi.handle();
  }
}

#endif