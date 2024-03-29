#ifndef _TASK_SENSORS_
#define _TASK_SENSORS_

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>

#include "global.h"
// #include "model/timeout_model.h"

void xTaskSensors(void *parameter) {
  Adafruit_ADS1115 ads(0x48);

  ads.setGain(GAIN_TWOTHIRDS);  // 2/3x gain +/- 6.144V  1 bit = 3mV      0.1875mV (default)
  ads.begin();

  temperatureSensor.setConversionCallback([](uint16_t value) {
    float voltage = value * 0.1875 / 1000;  // 0.1875V por LSB para o ADS1115
    float temperature = voltage * 100.0;    // 100mV por grau Celsius para o LM35
    // return temperature;
    return 100;
  });

  humiditySensor.setConversionCallback([](uint16_t value) {
    float voltage = value * 0.1875 / 1000;  // 0.1875V por LSB para o ADS1115
    float temperature = voltage * 100.0;    // 100mV por grau Celsius para o LM35
    float lm35_temperature_fahrenheit = (temperature * 9.0) / 5.0 + 32.0;
    // return lm35_temperature_fahrenheit;
    return 100;
  });

  // TimeoutModel debug(4000);

  while (1) {
    temperatureSensor.addValue(ads.readADC_SingleEnded(0));
    humiditySensor.addValue(ads.readADC_SingleEnded(1));

    // if (debug.complete()) {
    //   Serial.print("[DEBUG sensors] a0 = ");
    //   Serial.print(temperatureSensor.value(), 2);
    //   Serial.print("°C | ");
    //   Serial.print(humiditySensor.value(), 2);
    //   Serial.print("°F | ");
    //   Serial.println(temperatureSensor.complete() ? "done" : "reading");
    // }
    vTaskDelay(pdMS_TO_TICKS(200));
  }
}

#endif