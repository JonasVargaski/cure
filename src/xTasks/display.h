#ifndef _TASK_DISPLAY_
#define _TASK_DISPLAY_

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>

#include "global.h"

void xTaskDisplay(void *parameter)
{
  while (1)
  {
    vTaskDelay(pdMS_TO_TICKS(200));
  }
}

#endif