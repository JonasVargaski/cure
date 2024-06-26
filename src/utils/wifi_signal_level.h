#ifndef _WIFI_SIGNAL_LEVEL_UTIL_
#define _WIFI_SIGNAL_LEVEL_UTIL_
#include "Arduino.h"

unsigned int parseSignalLevel(int rssi) {
  if (rssi <= -100)
    return 0;

  if (rssi <= -85)
    return 1;

  if (rssi <= -75)
    return 2;

  if (rssi <= -60)
    return 3;

  return 4;
}

#endif