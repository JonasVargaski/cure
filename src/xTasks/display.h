#ifndef _TASK_DISPLAY_
#define _TASK_DISPLAY_

#include <Arduino.h>
#include <DWIN.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>

#include "global.h"

TickType_t xLastWakeTime = 0;

void onHMIEvent(DwinFrame *frame) {
  frame->print();

  uint16_t vp = frame->getVPAddress();

  if (vp == temperatureSetPoint.address())
    temperatureSetPoint.setValue(frame->getWorldValue());
  else if (vp == humiditySetPoint.address())
    humiditySetPoint.setValue(frame->getWorldValue());
  else if (vp == temperatureFanEnabled.address())
    temperatureFanEnabled.setValue(frame->getWorldValue());
  else if (vp == alarmEnabled.address())
    alarmEnabled.setValue(frame->getWorldValue());
  else if (vp == injectionScrewEnabled.address())
    injectionScrewEnabled.setValue(frame->getWorldValue());

  xLastWakeTime = 0;
}

void xTaskDisplay(void *parameter) {
  DWIN hmi(Serial2, 16, 17, 115200, 35);
  hmi.hmiCallBack(onHMIEvent);

  while (1) {
    if (xTaskGetTickCount() - xLastWakeTime >= pdMS_TO_TICKS(1000)) {
      hmi.setVPWord(temperatureSensor.address(), temperatureSensor.value());
      hmi.setVPWord(humiditySensor.address(), humiditySensor.value());
      hmi.setVPWord(temperatureSetPoint.address(), temperatureSetPoint.value());
      hmi.setVPWord(humiditySetPoint.address(), humiditySetPoint.value());
      hmi.setVPWord(temperatureFanEnabled.address(), temperatureFanEnabled.value());
      hmi.setVPWord(alarmEnabled.address(), alarmEnabled.value());
      hmi.setVPWord(injectionScrewEnabled.address(), injectionScrewEnabled.value());
      xLastWakeTime = xTaskGetTickCount();
    }

    hmi.handle();
    vTaskDelay(pdMS_TO_TICKS(60));
  }
}

#endif