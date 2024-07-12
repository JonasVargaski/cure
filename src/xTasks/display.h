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

TaskHandle_t xTaskDisplayHandle;

void xTaskDisplay(void* parameter) {
  Serial2.begin(115200, SERIAL_8N1, 16, 17);
  DwinT5II hmi(Serial2);

  hmi.setBrightness(90);
  hmi.setPage(1);
  hmi.beep(80);

  hmi.setCallback([](DataFrame* frame) {
    frame->print();

    uint16_t address = frame->getAddress();

    for (DisplayInt* obj : displayIntObjects) {
      if (address == obj->address) {
        obj->setValue(frame->getInt());
        break;
      }
    }

    for (DisplayText* obj : displayTextObjects) {
      if (address == obj->address) {
        obj->setValue(frame->getAscii().c_str());
        break;
      }
    }

    if (address == wifiSsidParam.address || address == wifiPasswordParam.address || address == remotePasswordParam.address) {
      restartWifiTask();
    }
  });

  while (1) {
    vTaskDelay(pdMS_TO_TICKS(200));

    hmi.setVP(temperatureSensor.address, temperatureSensor.isOutOfRange() ? 0 : temperatureSensor.value);
    hmi.setVP(humiditySensor.address, humiditySensor.isOutOfRange() ? 0 : humiditySensor.value);

    for (DisplayInt* obj : displayIntObjects)
      hmi.setVP(obj->address, obj->value);

    for (DisplayText* obj : displayTextObjects)
      hmi.setVP(obj->address, obj->value);
  }
}

#endif