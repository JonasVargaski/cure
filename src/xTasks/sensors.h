#ifndef _TASK_SENSORS_
#define _TASK_SENSORS_

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>

#include "global.h"

int adcToCelsius(uint16_t value) {
  float voltage = value * 0.1875 / 1000;
  float temperature = voltage * 100.0;
  return (int)temperature;
}

int adcToFahrenheit(uint16_t value) {
  float voltage = value * 0.1875 / 1000;
  float temperature = voltage * 100.0;
  float lm35_temperature_fahrenheit = (temperature * 9.0) / 5.0 + 32.0;
  return (int)(lm35_temperature_fahrenheit < 33 ? 0 : lm35_temperature_fahrenheit);
}

TaskHandle_t xTaskSensorsHandle;

void xTaskSensors(void *parameter) {
  Adafruit_ADS1115 ads(0x48);

  ads.setGain(GAIN_TWOTHIRDS);  // 2/3x gain +/- 6.144V  1 bit = 3mV      0.1875mV (default)
  ads.begin();

  temperatureSensor.setValidRange(33, 200);
  humiditySensor.setValidRange(33, 200);

  temperatureSensor.setConversionCallback(adcToCelsius);
  humiditySensor.setConversionCallback(adcToCelsius);

  while (1) {
    temperatureSensor.addValue(ads.readADC_SingleEnded(0));
    humiditySensor.addValue(ads.readADC_SingleEnded(1));
    vTaskDelay(pdMS_TO_TICKS(200));
  }
}

#endif