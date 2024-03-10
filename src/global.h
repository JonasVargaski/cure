#ifndef _SHARED_GLOBALS_
#define _SHARED_GLOBALS_

#include <Arduino.h>
#include <Preferences.h>
#include <Wire.h>

#include "model/moving-average.h"

#include "Adafruit_ADS1015.h"

#define pinCount 8

Preferences prefs;
SemaphoreHandle_t vpMutex;

MovingAverageModel sensor_temp(6);

// Pin map
int pins[pinCount] = {13, 12, 14, 27, 26, 25, 33, 32};

#endif