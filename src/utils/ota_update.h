#ifndef _OTA_UPDATE_UTILS_
#define _OTA_UPDATE_UTILS_

#include <HTTPClient.h>
#include <HTTPUpdate.h>
#include <WiFi.h>

#include "global.h"

void runOTAUpdate(const char* url) {
  Serial.println("[OTA] remote URL: " + String(url));

  if (connectionStatus.value() != 3) {
    Serial.println(F("[OTA] Not connected to WiFi, can't update firmware"));
    return;
  }

  Serial.println(F("[OTA] Starting update"));

  HTTPClient http;
  http.begin(url);

  if (Update.begin(UPDATE_SIZE_UNKNOWN)) {
    Serial.println(F("[OTA] Downloading..."));

    int httpCode = http.GET();
    if (httpCode == HTTP_CODE_OK) {
      WiFiClient& stream = http.getStream();
      uint8_t buffer[1024];
      int bytesRead;

      while ((bytesRead = stream.readBytes(buffer, sizeof(buffer))) > 0) {
        if (Update.write(buffer, bytesRead) != bytesRead) {
          Serial.println(F("[OTA] Error during OTA update bytes"));
          Update.end(false);
          return;
        }
      }

      if (Update.end(true)) {
        Serial.println(F("[OTA] update complete"));
        ESP.restart();
      } else {
        Serial.println(F("[OTA] Error during OTA update"));
        Update.end(false);
      }
    } else {
      Serial.println(F("[OTA] Failed to download firmware"));
      Update.end(false);
    }
  } else {
    Serial.println(F("[OTA] Failed to start update"));
  }

  http.end();
}

#endif