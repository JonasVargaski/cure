#ifndef _OTA_UPDATE_UTILS_
#define _OTA_UPDATE_UTILS_

#include <HTTPClient.h>
#include <Update.h>
#include <WiFi.h>
#include <esp_ota_ops.h>

#include "global.h"

bool updateFirmware(String url) {
  bool success = false;
  WiFiClientSecure client;
  client.setInsecure();
  HTTPClient http;
  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);

  Serial.println("STARTING OTA UPDATE");

  if (http.begin(client, url)) {
    int status = http.GET();
    log_i("Opening %s got %d.", url.c_str(), status);
    size_t size = http.getSize();
    log_i("http size: %d. ", size);
    log_i("Will access Stream:, free heap %dkb", ESP.getFreeHeap() / 1024);
    if (status == HTTP_CODE_OK) {
      Update.begin();
      Update.onProgress([size](size_t pos, size_t all) {
        Serial.println("Progress: " + String(pos) + " of " + String(all));
      });

      Stream& stream = http.getStream();
      byte buffer[256];
      size_t read;
      size_t written = 0;
      while ((read = stream.readBytes(&buffer[0], sizeof(buffer))) > 0) {
        Update.write(buffer, read);
        written += read;
      }
      log_i("Got %d bytes", written);
    } else {
      log_e("Got http status: %d", status);
      http.end();
      return false;
    }
  }
  http.end();
  if (Update.end(true)) {  // true to set the size to the current progress
    Serial.println("SUCCESS");

    // Suppress the ota Firmware switch!
    const esp_partition_t* running = esp_ota_get_running_partition();
    esp_ota_set_boot_partition(running);
    success = true;
  } else {
    Serial.println(Update.errorString());
  }
  return success;
}

#endif