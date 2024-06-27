#ifndef _OTA_UPDATE_UTILS_
#define _OTA_UPDATE_UTILS_

#include <HTTPClient.h>
#include <Update.h>
#include <WiFi.h>
#include <esp_ota_ops.h>

#include "global.h"

bool updateFirmware(String url) {
  WiFiClientSecure client;
  client.setInsecure();

  HTTPClient http;
  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);

  if (http.begin(client, url)) {
    int status = http.GET();
    Serial.printf("[OTA] Opening %s got %d\n", url.c_str(), status);
    size_t size = http.getSize();

    if (status == HTTP_CODE_OK) {
      Update.begin();
      Update.onProgress([size](size_t pos, size_t all) {
        float percentage = (float)pos / (float)all * 100.0;
        Serial.println("[OTA] Progress: " + String(percentage, 2) + "% | " + String(pos) + " of " + String(all));
      });

      Stream& stream = http.getStream();
      byte buffer[256];
      size_t read;
      size_t written = 0;
      while ((read = stream.readBytes(&buffer[0], sizeof(buffer))) > 0) {
        Update.write(buffer, read);
        written += read;
      }
      Serial.printf("[OTA] Got %d bytes\n", written);
    } else {
      Serial.printf("[OTA] Error: http status: %d\n", status);
      http.end();
      return false;
    }
  }

  http.end();

  if (Update.end(true) && Update.isFinished()) {
    Serial.println(F("[OTA] Update Success"));
    ESP.restart();
    return true;
  } else {
    Serial.print(F("[OTA] Update error: "));
    Serial.println(Update.errorString());
  }
  return false;
}

#endif