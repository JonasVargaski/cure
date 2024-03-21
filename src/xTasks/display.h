#ifndef _TASK_DISPLAY_
#define _TASK_DISPLAY_

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>

#include "global.h"
#include <DWIN.h>

TickType_t xLastWakeTime = 0;

void onHMIEvent(DwinFrame *frame)
{
  frame->print();

  uint16_t vp = frame->getVPAddress();

  if (vp == temperatureSetPoint.getVp())
    temperatureSetPoint.setValue(frame->getWorldValue());
  else if (vp == humiditySetPoint.getVp())
    humiditySetPoint.setValue(frame->getWorldValue());
  else if (vp == fanSetPoint.getVp())
    fanSetPoint.setValue(frame->getWorldValue());
  else if (vp == buzzerSetPoint.getVp())
    buzzerSetPoint.setValue(frame->getWorldValue());
  else if (vp == motorSetPoint.getVp())
    motorSetPoint.setValue(frame->getWorldValue());

  xLastWakeTime = 0;
}

void xTaskDisplay(void *parameter)
{
  DWIN hmi(Serial2, 16, 17, 115200, 35);
  hmi.hmiCallBack(onHMIEvent);

  while (1)
  {
    if (xTaskGetTickCount() - xLastWakeTime >= pdMS_TO_TICKS(1000))
    {
      hmi.setVPWord(temperatureSensor.getVp(), temperatureSensor.getValue());
      hmi.setVPWord(humiditySensor.getVp(), humiditySensor.getValue());
      hmi.setVPWord(temperatureSetPoint.getVp(), temperatureSetPoint.getValue());
      hmi.setVPWord(humiditySetPoint.getVp(), humiditySetPoint.getValue());
      hmi.setVPWord(fanSetPoint.getVp(), fanSetPoint.getValue());
      hmi.setVPWord(buzzerSetPoint.getVp(), buzzerSetPoint.getValue());
      hmi.setVPWord(motorSetPoint.getVp(), motorSetPoint.getValue());
      xLastWakeTime = xTaskGetTickCount();
    }

    hmi.handle();
    vTaskDelay(pdMS_TO_TICKS(60));
  }
}

#endif