#ifndef _SHARED_GLOBALS_
#define _SHARED_GLOBALS_

#include <Arduino.h>
#include <Preferences.h>
#include <Wire.h>

#include "Adafruit_ADS1015.h"
#include "model/bool_addressed_variable_model.h"
#include "model/sensor_input_moving_average_model.h"
#include "model/uint16_addressed_variable_model.h"

#define pinCount 8

// pin map output
#define alarmOutput 32
#define temperatureFanOutput 13
#define temperatureDamperAOutput 14
#define temperatureDamperBOutput 26
#define humidityDamperPwmOutput 15
#define humidityDamperA 27
#define humidityDamperB 25
#define injectionScrewA 12
#define injectionScrewB 33

SemaphoreHandle_t variableMutex;
SemaphoreHandle_t sensorMutex;

// Global variables
SensorInputAverageModel temperatureSensor(0x1001, &sensorMutex, 15);
SensorInputAverageModel humiditySensor(0x1003, &sensorMutex, 15);

Uint16StorageModel temperatureSetPoint(0x1002, &variableMutex, 70, 160);
Uint16StorageModel humiditySetPoint(0x1004, &variableMutex, 70, 140);

BoolStorageModel alarmEnabled(0x1006, &variableMutex);
Uint16StorageModel alarmDiffParam(0x2000, &variableMutex, 1, 20);      // diferença de temperatura e ajuste para ligar o alarme em ºC
Uint16StorageModel alarmReactiveParam(0x2001, &variableMutex, 1, 20);  // tempo para religar o alarme após desligado

BoolStorageModel temperatureFanEnabled(0x1005, &variableMutex);
BoolStorageModel injectionScrewEnabled(0x1007, &variableMutex);

// Pin map
int pins[pinCount] = {13, 12, 14, 27, 26, 25, 33, 32};

#endif