#ifndef _TASK_SENSORS_
#define _TASK_SENSORS_

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>

#include "global.h"

#define ADC_16BIT_MAX 65536

void xTaskSensors(void *parameter)
{
  Adafruit_ADS1115 ads(0x48);

  ads.setGain(GAIN_TWOTHIRDS); // 2/3x gain +/- 6.144V  1 bit = 3mV      0.1875mV (default)
  ads.begin();

  int count;

  while (1)
  {
    uint16_t analogValue = ads.readADC_SingleEnded(0);
    float voltage = analogValue * 0.1875 / 1000; // 0.1875V por LSB para o ADS1115

    float temperature = voltage * 100.0; // 100mV por grau Celsius para o LM35
    float lm35_temperature_fahrenheit = (temperature * 9.0) / 5.0 + 32.0;

    temperatureSensor.setValue(temperature);
    humiditySensor.setValue(lm35_temperature_fahrenheit);

    count++;

    // DEBUG SERIAL LEITURAS 10 secs
    if (count > 10)
    {
      count = 0;

      Serial.print("[DEBUG sensors] a0 = ");
      Serial.print(temperatureSensor.getValue());
      Serial.print(" | ");
      Serial.print(voltage, 3);
      Serial.print("V | ");
      Serial.print(temperature, 2);
      Serial.print("°C | ");
      Serial.print(lm35_temperature_fahrenheit, 2);
      Serial.println("°F");
    }

    vTaskDelay(pdMS_TO_TICKS(500));
  }
}

#endif