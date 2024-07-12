#ifndef _TASK_WIFI_
#define _TASK_WIFI_

#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <WiFi.h>

#include "enums.h"
#include "esp_task_wdt.h"
#include "global.h"
#include "model/async_timer_model.h"
#include "utils/array_utils.h"
#include "utils/ota_update.h"
#include "utils/wifi_signal_level.h"

#define MQTT_BROKER "broker.mqtt-dashboard.com"
#define MQTT_PORT 1883
#define MQTT_BUFFER_SIZE 600

TaskHandle_t xTaskWifiHandle = NULL;

void xTaskWifi(void* parameter) {
  WiFi.disconnect();
  bool registered = false;
  WiFiClient wifi;
  PubSubClient mqtt(wifi);

  AsyncTimerModel updateTimer;
  JsonDocument doc;

  String deviceId = WiFi.macAddress();
  deviceId.replace(":", "");
  deviceId.toLowerCase();

  static char MQTT_TOPIC_PARAMS[20];
  snprintf(MQTT_TOPIC_PARAMS, sizeof(MQTT_TOPIC_PARAMS), "/cure/%s", deviceId);

  WiFi.mode(WIFI_MODE_STA);
  WiFi.disconnect();
  wifiDeviceId.setValue(deviceId.c_str());

  mqtt.setBufferSize(MQTT_BUFFER_SIZE);
  mqtt.setServer(MQTT_BROKER, MQTT_PORT);
  mqtt.setCallback([](char* topic, byte* payload, unsigned int length) {
    Serial.println(F("\n[WIFI] Message received"));

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, payload, length);
    if (error) return;

    uint16_t blackListParams[] = {wifiDeviceId.address, firmwareVersion.address, remotePasswordParam.address, skipFactoryResetFlag.address};

    JsonArray data = doc["data"];
    for (JsonVariant item : data) {
      uint16_t address = item[0];

      if (address == eRemoteTriggerCallback::OTA_UPDATE) {
        updateFirmware(item[1]);
        continue;
      }

      if (arrayContains(address, blackListParams))
        continue;

      for (DisplayInt* obj : displayIntObjects) {
        if (obj->address == address) {
          obj->setValue(item[1]);
          bool updated = true;
          break;
        }
      }

      for (DisplayText* obj : displayTextObjects) {
        if (obj->address == address) {
          obj->setValue(item[1]);
          break;
        }
      }
    }
  });

  while (1) {
    while (WiFi.status() != WL_CONNECTED) {
      WiFi.begin(wifiSsidParam.value, wifiPasswordParam.value);
      wifiSignalQuality.setValue(0, false);
      connectionStatus.setValue(eWifiStatus::DISCONNECTED, false);
      vTaskDelay(pdMS_TO_TICKS(3000));
    }

    while (!mqtt.connected()) {
      wifiSignalQuality.setValue(parseSignalLevel(WiFi.RSSI()), false);
      connectionStatus.setValue(eWifiStatus::CONNECTING, false);
      vTaskDelay(pdMS_TO_TICKS(3000));

      if (mqtt.connect((String(random(0xffff), HEX) + String(random(0xffff), HEX)).c_str())) {
        char MQTT_TOPIC_RECEIVE_PARAMS[26];
        snprintf(MQTT_TOPIC_RECEIVE_PARAMS, sizeof(MQTT_TOPIC_RECEIVE_PARAMS), "/cure/%s:%04d", deviceId, remotePasswordParam.value);
        mqtt.subscribe(MQTT_TOPIC_RECEIVE_PARAMS);

        if (!registered) {
          doc.clear();
          JsonArray data = doc.to<JsonArray>();
          data.add(wifiDeviceId.value);
          data.add(MQTT_TOPIC_RECEIVE_PARAMS);
          data.add(workingTimeInHours.value);
          data.add(firmwareVersion.value);
          data.add(BUILD_TIME);

          static char buffer[100];
          serializeJson(doc, buffer);
          registered = mqtt.publish("/cure/on/register", buffer);
        }
      } else {
        connectionStatus.setValue(eWifiStatus::WITHOUT_INTERNET, false);
      }
    }

    if (updateTimer.waitFor(3000)) {
      updateTimer.reset();
      connectionStatus.setValue(eWifiStatus::CONNECTED, false);
      wifiSignalQuality.setValue(parseSignalLevel(WiFi.RSSI()), false);

      doc.clear();
      JsonArray data = doc["data"].to<JsonArray>();

      JsonArray tempSensor = data.add<JsonArray>();
      tempSensor.add(temperatureSensor.address);
      tempSensor.add(temperatureSensor.value);

      JsonArray humidSensor = data.add<JsonArray>();
      humidSensor.add(humiditySensor.address);
      humidSensor.add(humiditySensor.value);

      for (DisplayInt* obj : displayIntObjects) {
        JsonArray partial = data.add<JsonArray>();
        partial.add(obj->address);
        partial.add(obj->value);
      }

      for (DisplayText* obj : displayTextObjects) {
        JsonArray partial = data.add<JsonArray>();
        partial.add(obj->address);
        partial.add(obj->value);
      }

      JsonArray alarms = doc["alarm"].to<JsonArray>();
      for (int i = 1; i < eDisplayAlarm::DISPLAY_ALARM_MAX_SIZE - 1; i++) {
        alarms.add((int)activeAlarms[i]);
      }

      static char buffer[MQTT_BUFFER_SIZE];
      serializeJson(doc, buffer);
      mqtt.publish(MQTT_TOPIC_PARAMS, buffer);
    }

    mqtt.loop();
    vTaskDelay(pdMS_TO_TICKS(30));
  }
}

void restartWifiTask() {
  if (xTaskWifiHandle != NULL) {
    WiFi.disconnect();
    vTaskDelete(xTaskWifiHandle);
    xTaskWifiHandle = NULL;
  }

  Serial.println(F("wifiTask started"));
  xTaskCreatePinnedToCore(xTaskWifi, "wifiTask", 8048, NULL, 3, &xTaskWifiHandle, 1);
}

#endif