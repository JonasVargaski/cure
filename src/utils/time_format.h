#ifndef _TIME_FORMAT_UTIL_
#define _TIME_FORMAT_UTIL_
#include "Arduino.h"

void printFormattedTime(uint32_t milliseconds) {
  uint32_t seconds = milliseconds / 1000;
  uint32_t hours = seconds / 3600;
  uint32_t minutes = (seconds % 3600) / 60;
  uint32_t secs = seconds % 60;
  uint32_t millis = milliseconds % 1000;

  Serial.printf("%02d:%02d:%02d:%03d", hours, minutes, secs, millis);
}

#endif