#ifndef _TASK_CONTROL_
#define _TASK_CONTROL_

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>

#include "global.h"

void xTaskControl(void *parameter)
{
  while (1)
  {
    vTaskDelay(pdMS_TO_TICKS(100));
    digitalWrite(pins[1], HIGH);

    vTaskDelay(pdMS_TO_TICKS(100));
    digitalWrite(pins[1], LOW);

    digitalWrite(pins[3], !fanSetPoint.getValue());
    digitalWrite(pins[5], !buzzerSetPoint.getValue());
    digitalWrite(pins[7], !motorSetPoint.getValue());
  }
}

#endif