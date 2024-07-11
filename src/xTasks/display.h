#ifndef _TASK_DISPLAY_
#define _TASK_DISPLAY_

#include <Arduino.h>
#include <DwinT5II.h>
#include <WiFi.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>

#include "global.h"
#include "model/async_timer_model.h"
#include "xTasks/wifi.h"

void onHMIEvent(DataFrame* frame) {
  frame->print();

  uint16_t vp = frame->getAddress();
  bool updated = false;

  if (!updated) {
    for (Uint16StorageModel* obj : numberDisplayVariables) {
      if (obj->address() == vp) {
        updated = true;
        obj->setValueSync(frame->getInt());
        break;
      }
    }
  }

  if (!updated) {
    for (BoolStorageModel* obj : booleanDisplayVariables) {
      if (obj->address() == vp) {
        updated = true;
        obj->setValueSync(frame->getInt());
        break;
      }
    }
  }
  if (!updated) {
    for (TextStorageModel* obj : textDisplayVariables) {
      if (obj->address() == vp) {
        updated = true;
        obj->setValueSync(frame->getAscii().c_str());
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
  Serial2.begin(115200, SERIAL_8N1, 16, 17);
  DwinT5II hmi(Serial2);

  hmi.setCallback(onHMIEvent);
  hmi.setBrightness(90);
  hmi.setPage(1);

  while (1) {
    vTaskDelay(pdMS_TO_TICKS(300));

    hmi.setVP(temperatureSensor.address(), temperatureSensor.isOutOfRange() ? 0 : temperatureSensor.value());
    hmi.setVP(humiditySensor.address(), humiditySensor.isOutOfRange() ? 0 : humiditySensor.value());

    for (Uint16StorageModel* obj : numberDisplayVariables)
      hmi.setVP(obj->address(), obj->value());

    for (BoolStorageModel* obj : booleanDisplayVariables)
      hmi.setVP(obj->address(), obj->value());

    for (TextStorageModel* obj : textDisplayVariables)
      hmi.setVP(obj->address(), obj->value());
  }
}

#endif