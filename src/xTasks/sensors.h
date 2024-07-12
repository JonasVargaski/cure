#ifndef _TASK_SENSORS_
#define _TASK_SENSORS_

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>

#include "global.h"

float adcToVoltage(uint16_t value) {  // 2/3x gain +/- 6.144V  1 bit = 3mV      0.1875mV (default)
  return value * 0.1875 / 1000;       // 0.1875 mV per bit, converted to V
}

float voltageToCelsius(float voltage) {
  float temperature = voltage * 100.0;
  return temperature;
}

float voltageToFahrenheit(float voltage) {
  float temperature = voltage * 100.0;
  float lm35_temperature_fahrenheit = (temperature * 9.0) / 5.0 + 32.0;
  return (lm35_temperature_fahrenheit < 33 ? 0 : lm35_temperature_fahrenheit);
}

float voltageToHumidity(float voltage, float temperature = 25) {
  float sensorRH = (voltage - 0.826) / 0.0315;
  float trueRH = sensorRH / (1.0546 - (0.00216 * temperature));
  return trueRH;
}

TaskHandle_t xTaskSensorsHandle;

void xTaskSensors(void *parameter) {
  Adafruit_ADS1115 ads(0x48);

  ads.setGain(GAIN_TWOTHIRDS);  // 2/3x gain +/- 6.144V  1 bit = 3mV      0.1875mV (default)
  ads.begin();

  while (1) {
    vTaskDelay(150);
    float vCN0 = adcToVoltage(ads.readADC_SingleEnded(0));
    if (temperatureSensorType.value) {  // °C
      temperatureSensor.setRange(3, 120);
      temperatureSensor.setValue((int)voltageToCelsius(vCN0));
    } else {  // °F
      temperatureSensor.setRange(33, 200);
      temperatureSensor.setValue((int)voltageToFahrenheit(vCN0));
    }

    vTaskDelay(150);
    float vCN1 = adcToVoltage(ads.readADC_SingleEnded(1));
    if (humiditySensorType.value) {  // UR%
      humiditySensor.setRange(3, 100);
      humiditySensor.setValue((int)voltageToHumidity(vCN1, voltageToCelsius(vCN0)));
    } else {  // °F
      humiditySensor.setRange(33, 200);
      humiditySensor.setValue((int)voltageToFahrenheit(vCN1));
    }
  }
}

#endif