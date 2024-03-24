#ifndef _SHARED_GLOBALS_
#define _SHARED_GLOBALS_

#include <Arduino.h>
#include <Preferences.h>
#include <Wire.h>

#include "Adafruit_ADS1015.h"
#include "model/bool_addressed_variable_model.h"
#include "model/sensor_input_moving_average_model.h"
#include "model/text_addressed_variable_model.h"
#include "model/uint16_addressed_variable_model.h"
#include "vector"

SemaphoreHandle_t variableMutex;
SemaphoreHandle_t sensorMutex;
SemaphoreHandle_t outputMutex;

// Global variables
SensorInputAverageModel temperatureSensor(0x1001, &sensorMutex, 15);
SensorInputAverageModel humiditySensor(0x1003, &sensorMutex, 15);

Uint16StorageModel temperatureSetPoint(0x1002, &variableMutex, 70, 160);
Uint16StorageModel humiditySetPoint(0x1004, &variableMutex, 70, 140);

BoolStorageModel alarmEnabled(0x1006, &variableMutex);
Uint16StorageModel alarmReactiveParam(0x1300, &variableMutex, 0, 20);  // [x (in ms) * 1000 * 60 = Minutes]
Uint16StorageModel alarmHumidityDiffParam(0x1301, &variableMutex, 1, 25);
Uint16StorageModel alarmTemperatureDiffParam(0x1302, &variableMutex, 1, 25);

BoolStorageModel alarmOutputState(0x1015, &outputMutex);  // definir VP

BoolStorageModel temperatureFanEnabled(0x1005, &variableMutex);
BoolStorageModel injectionScrewEnabled(0x1007, &variableMutex);

TextStorageModel wifiSsidParam(0x1100, &variableMutex, 20);
TextStorageModel wifiPasswordParam(0x1150, &variableMutex, 20);

std::vector<Uint16StorageModel*> uint16StorageVariables = {
    &temperatureSetPoint,
    &humiditySetPoint,
    &alarmReactiveParam,
    &alarmHumidityDiffParam,
    &alarmTemperatureDiffParam,
};

std::vector<BoolStorageModel*> boolStorageVariables = {
    &alarmEnabled,
    &alarmOutputState,
    &temperatureFanEnabled,
    &injectionScrewEnabled,
};

std::vector<TextStorageModel*> textStorageVariables = {
    &wifiSsidParam,
    &wifiPasswordParam,
};

#endif