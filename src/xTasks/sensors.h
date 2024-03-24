#ifndef _TASK_SENSORS_
#define _TASK_SENSORS_

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>

#include "global.h"

#define ADC_16BIT_MAX 65536

void xTaskSensors(void *parameter) {
  Adafruit_ADS1115 ads(0x48);

  ads.setGain(GAIN_TWOTHIRDS);  // 2/3x gain +/- 6.144V  1 bit = 3mV      0.1875mV (default)
  ads.begin();

  temperatureSensor.setConversionCallback([](uint16_t value) {
    float voltage = value * 0.1875 / 1000;  // 0.1875V por LSB para o ADS1115
    float temperature = voltage * 100.0;    // 100mV por grau Celsius para o LM35
    return temperature;
  });

  humiditySensor.setConversionCallback([](uint16_t value) {
    float voltage = value * 0.1875 / 1000;  // 0.1875V por LSB para o ADS1115
    float temperature = voltage * 100.0;    // 100mV por grau Celsius para o LM35
    float lm35_temperature_fahrenheit = (temperature * 9.0) / 5.0 + 32.0;
    return lm35_temperature_fahrenheit;
  });

  TickType_t xLastWakeTime = 0;

  while (1) {
    uint16_t analogValue = ads.readADC_SingleEnded(0);

    temperatureSensor.addValue(analogValue);
    humiditySensor.addValue(analogValue);

    if (xTaskGetTickCount() - xLastWakeTime >= pdMS_TO_TICKS(6000)) {
      Serial.print("[DEBUG sensors] a0 = ");
      Serial.print(temperatureSensor.value(), 2);
      Serial.print("°C | ");
      Serial.print(humiditySensor.value(), 2);
      Serial.print("°F | ");
      Serial.println(temperatureSensor.isComplete() ? "done" : "reading");
      xLastWakeTime = xTaskGetTickCount();
    }

    vTaskDelay(pdMS_TO_TICKS(200));
  }
}

#endif