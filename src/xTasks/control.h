#ifndef _TASK_CONTROL_
#define _TASK_CONTROL_

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>

void xTaskControl(void *parameter)
{
  while (1)
  {
    vTaskDelay(pdMS_TO_TICKS(300));
    digitalWrite(pins[2], HIGH);

    vTaskDelay(pdMS_TO_TICKS(300));
    digitalWrite(pins[2], LOW);
  }
}

#endif