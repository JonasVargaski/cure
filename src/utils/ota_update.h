#ifndef _OTA_UPDATE_UTILS_
#define _OTA_UPDATE_UTILS_

#include <HTTPUpdate.h>
#include <WiFi.h>

#include "defines/version.h"

void updateFirmware(String url) {
  WiFiClient client;

  Serial.print(F("OTA: update started, url: "));
  Serial.println(url);

  httpUpdate.onProgress([](int cur, int total) {
    Serial.printf("OTA: update progress %d of %d bytes\n", cur, total);
  });

  httpUpdate.onError([](int err) {
    Serial.printf("OTA: update fatal error code %d\n", err);
  });

  httpUpdate.onEnd([]() {
    Serial.println(F("OTA: update process finished"));
  });

  t_httpUpdate_return ret = httpUpdate.update(client, url, FIRMWARE_VERSION);

  switch (ret) {
    case HTTP_UPDATE_FAILED:
      Serial.printf("OTA: update failed. Error (%d): %s\n", httpUpdate.getLastError(), httpUpdate.getLastErrorString().c_str());
      break;

    case HTTP_UPDATE_NO_UPDATES:
      Serial.println(F("OTA: device is already up to date."));
      break;
  }
}

#endif