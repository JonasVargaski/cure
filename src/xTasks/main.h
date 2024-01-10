#ifndef _TASK_MAIN_
#define _TASK_MAIN_

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>

#include "global.h"

void xTaskMain(void *parameter)
{
  pinMode(13, OUTPUT);
  Serial.begin(115200);

  readTemp.begin();
  readTemp.setValue(1234);

  while (1)
  {

    if (digitalRead(13))
    {
      digitalWrite(13, 0);
    }
    else
    {
      digitalWrite(13, 1);
    }

    Serial.print("a0 = ");
    Serial.print(ads.readADC(0));
    Serial.print(" | ");
    Serial.print(sensor_temp.value());
    Serial.println(sensor_temp.isComplete() ? " | done" : " | waiting");

    Serial.print("vp = ");
    Serial.print(readTemp.vp);
    Serial.print(" | ");
    Serial.println(readTemp.value());

    vTaskDelay(pdMS_TO_TICKS(300));
  }
}

#endif