#ifndef _TASK_MAIN_
#define _TASK_MAIN_

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>

#include "global.h"

#define ADC_16BIT_MAX 65536

void xTaskMain(void *parameter)
{

  while (1)
  {
    int16_t adc0; // Variável para armazenar o valor lido do ADS1115

    sensor_temp.setValue(ads.readADC_SingleEnded(0)); // Lê o valor do canal 0

    // Converte o valor lido para tensão e, em seguida, para temperatura em graus Celsius
    float voltage = sensor_temp.value() * 0.1875 / 1000; // 0.1875V por LSB para o ADS1115
    float temperature = voltage * 100.0;                 // 100mV por grau Celsius para o LM35
    float lm35_temperature_fahrenheit = (temperature * 9.0) / 5.0 + 32.0;

    Serial.print("Tensão: ");
    Serial.print(voltage, 3);
    Serial.print("V  ");

    Serial.print("Temperatura: ");
    Serial.print(temperature, 2);
    Serial.print("°C  ");

    Serial.print("Temperatura: ");
    Serial.print(lm35_temperature_fahrenheit, 2);
    Serial.println("°F");

    // Converte o valor lido para tensão e, em seguida, para temperatura em graus Celsius
    adc0 = ads.readADC_SingleEnded(0);
    float voltage2 = adc0 * 0.1875 / 1000; // 0.1875V por LSB para o ADS1115
    float temperature2 = voltage2 * 100.0; // 100mV por grau Celsius para o LM35
    float lm35_temperature_fahrenheit2 = (temperature2 * 9.0) / 5.0 + 32.0;

    Serial.print("Tensão: ");
    Serial.print(voltage2, 3);
    Serial.print("V  ");

    Serial.print("Temperatura: ");
    Serial.print(temperature2, 2);
    Serial.print("°C  ");

    Serial.print("Temperatura: ");
    Serial.print(lm35_temperature_fahrenheit2, 2);
    Serial.println("°F **\n");

    vTaskDelay(pdMS_TO_TICKS(500));
  }
}

#endif
