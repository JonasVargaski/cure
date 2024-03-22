#ifndef _SHARED_GLOBALS_
#define _SHARED_GLOBALS_

#include <Arduino.h>
#include <Preferences.h>
#include <Wire.h>

#include "Adafruit_ADS1015.h"
#include "model/moving-average.h"
#include "model/persistent-variable.h"

#define pinCount 8

SemaphoreHandle_t variableMutex;

// Global variables
MovingAverageModel temperatureSensor(0x1002, 6);
MovingAverageModel humiditySensor(0x1006, 6);

// Globals Parameters

PersistentVariable temperatureSetPoint(0x1001, 70, 160, &variableMutex);
PersistentVariable humiditySetPoint(0x1007, 70, 140, &variableMutex);

PersistentVariable fanSetPoint(0x1003, 0, 1, &variableMutex);
PersistentVariable buzzerSetPoint(0x1004, 0, 1, &variableMutex);
PersistentVariable motorSetPoint(0x1005, 0, 1, &variableMutex);

// Pin map
int pins[pinCount] = {13, 12, 14, 27, 26, 25, 33, 32};

#endif