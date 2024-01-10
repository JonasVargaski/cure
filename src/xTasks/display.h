#ifndef _TASK_DISPLAY_
#define _TASK_DISPLAY_

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>

#include "global.h"

void xTaskDisplay(void *parameter)
{
  pinMode(14, OUTPUT);

  while (1)
  {
    // if (xSemaphoreTake(displayMutex, pdMS_TO_TICKS(200))) // Aguarda 200ms ou segue execução
    // {
    //   xSemaphoreGive(displayMutex);
    // }

    if (digitalRead(14))
    {
      digitalWrite(14, 0);
    }
    else
    {
      digitalWrite(14, 1);
    }

    vTaskDelay(pdMS_TO_TICKS(200));
  }
}

#endif