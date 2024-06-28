#ifndef _TASK_DISPLAY_
#define _TASK_DISPLAY_

#include <Arduino.h>
#include <DWIN.h>
#include <WiFi.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>

#include "global.h"

void onHMIEvent(DwinFrame* frame) {
  frame->print();

  uint16_t vp = frame->getVPAddress();

  for (Uint16StorageModel* obj : numberDisplayVariables) {
    if (obj->address() == vp) {
      obj->resetTimeUpdate();
      return obj->setValueSync(frame->getWorldValue());
    }
  }
  for (BoolStorageModel* obj : booleanDisplayVariables) {
    if (obj->address() == vp) {
      obj->resetTimeUpdate();
      return obj->setValueSync(frame->getWorldValue());
    }
  }
  for (TextStorageModel* obj : textDisplayVariables) {
    if (obj->address() == vp) {
      obj->resetTimeUpdate();
      return obj->setValueSync(frame->getTextValue().c_str());
    }
  }

  if (vp == wifiSsidParam.address() || vp == wifiPasswordParam.address()) {
    WiFi.disconnect();
  }
}

TaskHandle_t xTaskDisplayHandle;

void xTaskDisplay(void* parameter) {
  DWIN hmi(Serial2, 16, 17, 115200, 35);
  hmi.hmiCallBack(onHMIEvent);
  hmi.setBrightness(60);

  TickType_t xLastWakeTime = 0;

  while (1) {
    if (xTaskGetTickCount() - xLastWakeTime >= pdMS_TO_TICKS(1000)) {
      hmi.setVPWord(temperatureSensor.address(), temperatureSensor.value());
      hmi.setVPWord(humiditySensor.address(), humiditySensor.value());
      xLastWakeTime = xTaskGetTickCount();
    }

    for (Uint16StorageModel* obj : numberDisplayVariables) {
      if (obj->shouldUpdateDisplay())
        hmi.setVPWord(obj->address(), obj->value());
    }
    for (BoolStorageModel* obj : booleanDisplayVariables) {
      if (obj->shouldUpdateDisplay())
        hmi.setVPWord(obj->address(), obj->value());
    }
    for (TextStorageModel* obj : textDisplayVariables) {
      if (obj->shouldUpdateDisplay())
        hmi.setText(obj->address(), obj->value());
    }

    hmi.handle();
    vTaskDelay(pdMS_TO_TICKS(60));
  }
}

#endif