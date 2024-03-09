#ifndef _SHARED_GLOBALS_
#define _SHARED_GLOBALS_

#include <Arduino.h>
#include <Preferences.h>
#include <Wire.h>

#include "model/moving-average.h"
#include "model/display-int16t.h"

#include "Adafruit_ADS1015.h"

#define pinCount 8

Preferences prefs;
SemaphoreHandle_t vpMutex;

MovingAverageModel sensor_temp(6);
DisplayInt16Model readTemp(1000, &vpMutex, &prefs);

Adafruit_ADS1115 ads(0x48);

// Pin map
int pins[pinCount] = {13, 12, 14, 27, 26, 25, 33, 32};

#endif