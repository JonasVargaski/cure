#ifndef _TASK_WIFI_
#define _TASK_WIFI_

#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <WiFi.h>

#include "enums.h"
#include "global.h"
#include "model/async_timer_model.h"
#include "utils/array_utils.h"
#include "utils/device_id.h"
#include "utils/ota_update.h"
#include "utils/wifi_signal_level.h"

#define MQTT_BROKER "broker.mqtt-dashboard.com"
#define MQTT_PORT 1883
#define MQTT_BUFFER_SIZE 512

TaskHandle_t xTaskWifiHandle;

void xTaskWifi(void* parameter) {
  bool registered = false;
  WiFiClient wifi;
  PubSubClient mqtt(wifi);

  AsyncTimerModel sendParamsTimer;
  JsonDocument doc;

  struct MqttTopics {
    char params[20];
    char receiveParams[26];

    MqttTopics() {
      String deviceId = getDeviceId();
      snprintf(params, sizeof(params), "/cure/%s", deviceId);
      snprintf(receiveParams, sizeof(receiveParams), "/cure/%s:%04d", deviceId, remotePasswordParam.value());
    }
  } mqttTopic;

  WiFi.mode(WIFI_MODE_STA);
  wifiDeviceId.setValueSync(getDeviceId().c_str());

  mqtt.setBufferSize(MQTT_BUFFER_SIZE);
  mqtt.setServer(MQTT_BROKER, MQTT_PORT);
  mqtt.setCallback([mqttTopic](char* topic, byte* payload, unsigned int length) {
    Serial.print(F("[WIFI] Message received"));

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, payload, length);
    if (error) return;

    uint16_t blackListParams[] = {
        alarmOutputState.address(), temperatureFanOutputState.address(), humidityDamperOutputState.address(), injectionMachineOutputState.value(),
        wifiDeviceId.address(), firmwareVersion.address(), remotePasswordParam.address(), connectionStatus.address(), wifiSignalQuality.address()};

    JsonArray data = doc["data"];
    for (JsonVariant item : data) {
      uint16_t address = item[0];

      if (address == eRemoteTriggerCallback::OTA_UPDATE) {
        updateFirmware(item[1]);
        continue;
      }

      if (checkIfNumberExists(address, blackListParams))
        continue;

      bool updated = false;
      for (Uint16StorageModel* obj : numberDisplayVariables) {
        if (obj->address() == address) {
          obj->resetTimeUpdate();
          obj->setValueSync(item[1]);
          bool updated = true;
          break;
        }
      }

      if (updated) continue;
      for (BoolStorageModel* obj : booleanDisplayVariables) {
        if (obj->address() == address) {
          obj->resetTimeUpdate();
          obj->setValueSync(item[1]);
          bool updated = true;
          break;
        }
      }

      if (updated) continue;
      for (TextStorageModel* obj : textDisplayVariables) {
        if (obj->address() == address) {
          obj->resetTimeUpdate();
          obj->setValueSync(item[1]);
          break;
        }
      }
    }
  });

  while (1) {
    while (WiFi.status() != WL_CONNECTED) {
      WiFi.begin(wifiSsidParam.value(), wifiPasswordParam.value());
      wifiSignalQuality.setValueSync(0, false);
      connectionStatus.setValueSync(WiFi.status() == WL_CONNECT_FAILED ? eWifiStatus::DISCONNECTED : eWifiStatus::CONNECTING, false);
      vTaskDelay(pdMS_TO_TICKS(3000));
    }

    while (!mqtt.connected()) {
      vTaskDelay(pdMS_TO_TICKS(20));
      if (mqtt.connect((String(random(0xffff), HEX) + String(random(0xffff), HEX)).c_str())) {
        mqtt.subscribe(mqttTopic.receiveParams);
        if (!registered) {
          doc.clear();
          JsonArray data = doc.to<JsonArray>();
          data.add(wifiDeviceId.value());
          data.add(remotePasswordParam.value());
          data.add(mqttTopic.receiveParams);
          data.add(firmwareVersion.value());
          data.add(BUILD_TIME);

          char buffer[128];
          size_t jsonLength = serializeJson(doc, buffer);
          registered = mqtt.publish("/cure/on/register", buffer, jsonLength);
        }
      } else {
        connectionStatus.setValueSync(eWifiStatus::WITHOUT_INTERNET, false);
      }
    }

    if (sendParamsTimer.waitFor(3000)) {
      sendParamsTimer.reset();
      connectionStatus.setValueSync(eWifiStatus::CONNECTED, false);
      wifiSignalQuality.setValueSync(parseSignalLevel(WiFi.RSSI()), false);

      doc.clear();
      JsonArray data = doc["data"].to<JsonArray>();

      JsonArray tempSensor = data.add<JsonArray>();
      tempSensor.add(temperatureSensor.address());
      tempSensor.add(temperatureSensor.value());

      JsonArray humidSensor = data.add<JsonArray>();
      humidSensor.add(humiditySensor.address());
      humidSensor.add(humiditySensor.value());

      for (Uint16StorageModel* obj : numberDisplayVariables) {
        JsonArray partial = data.add<JsonArray>();
        partial.add(obj->address());
        partial.add(obj->value());
      }
      for (BoolStorageModel* obj : booleanDisplayVariables) {
        JsonArray partial = data.add<JsonArray>();
        partial.add(obj->address());
        partial.add((int)obj->value());
      }
      for (TextStorageModel* obj : textDisplayVariables) {
        JsonArray partial = data.add<JsonArray>();
        partial.add(obj->address());
        partial.add(obj->value());
      }

      JsonArray alarms = doc["alarm"].to<JsonArray>();
      alarms.add((int)alarmFlags.ELECTRICAL_SUPPLY);
      alarms.add((int)alarmFlags.VENTILATION_FAILURE);
      alarms.add((int)alarmFlags.TEMPERATURE_SENSOR_FAILURE);
      alarms.add((int)alarmFlags.HUMIDITY_SENSOR_FAILURE);
      alarms.add((int)alarmFlags.SECURITY_MODE_HIGH);
      alarms.add((int)alarmFlags.SECURITY_MODE_LOW);
      alarms.add((int)alarmFlags.TEMPERATURE_LOW);
      alarms.add((int)alarmFlags.TEMPERATURE_HIGH);
      alarms.add((int)alarmFlags.HUMIDITY_LOW);
      alarms.add((int)alarmFlags.HUMIDITY_HIGH);

      char buffer[MQTT_BUFFER_SIZE];
      size_t jsonLength = serializeJson(doc, buffer);
      mqtt.publish(mqttTopic.params, buffer, jsonLength);
    }

    mqtt.loop();
    vTaskDelay(pdMS_TO_TICKS(30));
  }
}

#endif