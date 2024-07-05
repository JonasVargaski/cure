#ifndef _TASK_DISPLAY_
#define _TASK_DISPLAY_

#include <Arduino.h>
#include <DWIN.h>
#include <WiFi.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>

#include "global.h"
#include "xTasks/wifi.h"

void onHMIEvent(DwinFrame* frame) {
  frame->print();

  uint16_t vp = frame->getVPAddress();
  bool updated = false;

  if (!updated) {
    for (Uint16StorageModel* obj : numberDisplayVariables) {
      if (obj->address() == vp) {
        updated = true;
        obj->resetTimeUpdate();
        obj->setValueSync(frame->getWorldValue());
        break;
      }
    }
  }
  if (!updated) {
    for (BoolStorageModel* obj : booleanDisplayVariables) {
      if (obj->address() == vp) {
        updated = true;
        obj->resetTimeUpdate();
        obj->setValueSync(frame->getWorldValue());
        break;
      }
    }
  }
  if (!updated) {
    for (TextStorageModel* obj : textDisplayVariables) {
      if (obj->address() == vp) {
        updated = true;
        obj->resetTimeUpdate();
        obj->setValueSync(frame->getTextValue().c_str());
        break;
      }
    }
  }

  if (vp == wifiSsidParam.address() || vp == wifiPasswordParam.address() || vp == remotePasswordParam.address()) {
    restartWifiTask();
  }
}

TaskHandle_t xTaskDisplayHandle;

void xTaskDisplay(void* parameter) {
  DWIN hmi(Serial2, 16, 17, 115200, 35);
  hmi.hmiCallBack(onHMIEvent);
  hmi.setBrightness(100);
  hmi.setPage(1);

  TickType_t xLastWakeTime = 0;

  while (1) {
    if (xTaskGetTickCount() - xLastWakeTime >= pdMS_TO_TICKS(1000)) {
      hmi.setVPWord(temperatureSensor.address(), temperatureSensor.isOutOfRange() ? 0 : temperatureSensor.value());
      hmi.setVPWord(humiditySensor.address(), humiditySensor.isOutOfRange() ? 0 : humiditySensor.value());
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