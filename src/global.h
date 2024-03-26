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

// Alarm flags
struct AlarmTypeFlag {
  bool ELECTRICAL_SUPPLY;
  bool VENTILATION_FAILURE;
  bool TEMPERATURE_SENSOR_FAILURE;
  bool HUMIDITY_SENSOR_FAILURE;
  bool SECURITY_MODE_HI;
  bool SECURITY_MODE_LOW;
  bool TEMPERATURE_LOW;
  bool TEMPERATURE_HIGH;
  bool HUMIDITY_LOW;
  bool HUMIDITY_HIGH;
} alarmFlags;

// Global variables
SensorInputAverageModel temperatureSensor(0x1001, &sensorMutex, 15);
SensorInputAverageModel humiditySensor(0x1003, &sensorMutex, 15);

Uint16StorageModel temperatureSetPoint(0x1002, &variableMutex, 70, 160);
Uint16StorageModel humiditySetPoint(0x1004, &variableMutex, 70, 140);

BoolStorageModel alarmEnabled(0x1006, &variableMutex);
Uint16StorageModel alarmReactiveParam(0x1300, &variableMutex, 0, 240);  // [x (in ms) * 1000 * 60 = Minutes]
Uint16StorageModel alarmHumidityDiffParam(0x1301, &variableMutex, 3, 30);
Uint16StorageModel alarmTemperatureDiffParam(0x1302, &variableMutex, 3, 30);

Uint16StorageModel alarmTemperatureTypeParam(0x1303, &variableMutex, 0, 3);  // enums.eAlarmType
Uint16StorageModel alarmHumidityTypeParam(0x1304, &variableMutex, 0, 3);     // enums.eAlarmType
Uint16StorageModel alarmSecurityTypeParam(0x1305, &variableMutex, 0, 3);     // enums.eAlarmType
BoolStorageModel alarmVentilationTypeParam(0x1306, &variableMutex);          // ON - OF
BoolStorageModel alarmElectricalSupplyTypeParam(0x1307, &variableMutex);     // ON - OF

BoolStorageModel alarmOutputState(0x1015, &outputMutex);  // definir VP

BoolStorageModel temperatureFanEnabled(0x1005, &variableMutex);
BoolStorageModel injectionScrewEnabled(0x1007, &variableMutex);

TextStorageModel wifiSsidParam(0x4000, &variableMutex, 20);
TextStorageModel wifiPasswordParam(0x1150, &variableMutex, 20);

std::vector<Uint16StorageModel*> uint16StorageVariables = {
    &temperatureSetPoint,
    &humiditySetPoint,
    &alarmReactiveParam,
    &alarmHumidityDiffParam,
    &alarmTemperatureDiffParam,
    &alarmTemperatureTypeParam,
    &alarmHumidityTypeParam,
    &alarmSecurityTypeParam,
};

std::vector<BoolStorageModel*> boolStorageVariables = {
    &alarmEnabled,
    &alarmOutputState,
    &temperatureFanEnabled,
    &injectionScrewEnabled,
    &alarmVentilationTypeParam,
    &alarmElectricalSupplyTypeParam,
};

std::vector<TextStorageModel*> textStorageVariables = {
    &wifiSsidParam,
    &wifiPasswordParam,
};

#endif