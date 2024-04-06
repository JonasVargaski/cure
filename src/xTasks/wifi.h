#ifndef _TASK_WIFI_
#define _TASK_WIFI_

#include <PubSubClient.h>
#include <WiFi.h>

#include "enums.h"
#include "global.h"
#include "model/async_timer_model.h"
#include "utils/wifi_signal_level.h"

#define MQTT_BROKER "broker.mqtt-dashboard.com"
#define MQTT_PORT 1883

void onReceived(char *topic, byte *payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void xTaskWifi(void *parameter) {
  WiFiClient wifi;
  PubSubClient mqtt(wifi);

  AsyncTimerModel sendParamsTimer;

  String deviceId = WiFi.macAddress();
  deviceId.replace(":", "");
  deviceId.toUpperCase();

  wifiDeviceId.setValueSync(deviceId.c_str());

  WiFi.mode(WIFI_MODE_STA);

  mqtt.setServer(MQTT_BROKER, MQTT_PORT);
  mqtt.setCallback(onReceived);

  while (1) {
    while (WiFi.status() != WL_CONNECTED) {
      WiFi.begin(wifiSsidParam.value(), wifiPasswordParam.value());
      wifiSignalQuality.setValueSync(0, false);
      connectionStatus.setValueSync(WiFi.status() == WL_CONNECT_FAILED ? eWifiStatus::DISCONNECTED : eWifiStatus::CONNECTING, false);
      vTaskDelay(pdMS_TO_TICKS(3000));
    }

    while (!mqtt.connected()) {
      if (mqtt.connect((String(random(0xffff), HEX) + String(random(0xffff), HEX)).c_str())) {
        char receiveTopic[15];
        sprintf(receiveTopic, "%s-%04d", wifiDeviceId.value(), remotePasswordParam.value());
        mqtt.subscribe(receiveTopic);
        Serial.printf("listening on topic: %s\n", receiveTopic);
      } else {
        connectionStatus.setValueSync(eWifiStatus::WITHOUT_INTERNET, false);
      }
      vTaskDelay(pdMS_TO_TICKS(20));
    }

    if (sendParamsTimer.waitFor(3000)) {
      sendParamsTimer.reset();
      connectionStatus.setValueSync(eWifiStatus::CONNECTED, false);
      wifiSignalQuality.setValueSync(parseSignalLevel(WiFi.RSSI()), false);

      Serial.printf("[WIFI] id: %s, signal: %d \n", wifiDeviceId.value(), wifiSignalQuality.value());
      mqtt.publish("test-jonas", "ABC123");
    }

    mqtt.loop();
    vTaskDelay(pdMS_TO_TICKS(30));
  }
}

#endif