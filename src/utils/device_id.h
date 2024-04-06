#ifndef _DEVICE_ID_
#define _DEVICE_ID_
#include <WiFi.h>

String getDeviceId() {
  String deviceId = WiFi.macAddress();
  deviceId.replace(":", "");
  deviceId.toLowerCase();
  return deviceId;
}
#endif