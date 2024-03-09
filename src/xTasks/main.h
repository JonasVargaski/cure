#ifndef _TASK_MAIN_
#define _TASK_MAIN_

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>

#include "global.h"

void xTaskMain(void *parameter)
{

  while (1)
  {
    Serial.print("adc 0 = ");
    // Serial.print(ads.readADC(0));
    Serial.print(" | ");
    Serial.print(sensor_temp.value());
    Serial.println(sensor_temp.isComplete() ? " | done" : " | waiting");

    // sensor_temp.setValue(ads.readADC(0));

    vTaskDelay(pdMS_TO_TICKS(500));
  }
}

#endif