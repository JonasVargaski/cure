#ifndef _TASK_CONTROL_
#define _TASK_CONTROL_

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>

void xTaskControl(void *parameter)
{
  pinMode(26, OUTPUT);

  while (1)
  {

    // if (xSemaphoreTake(displayMutex, portMAX_DELAY))
    // {
    //   xSemaphoreGive(displayMutex);
    // }

    if (digitalRead(26))
    {
      digitalWrite(26, 0);
    }
    else
    {
      digitalWrite(26, 1);
    }

    vTaskDelay(pdMS_TO_TICKS(300));
  }
}

#endif