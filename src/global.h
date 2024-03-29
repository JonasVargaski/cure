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
  bool SECURITY_MODE_HIGH;
  bool SECURITY_MODE_LOW;
  bool TEMPERATURE_LOW;
  bool TEMPERATURE_HIGH;
  bool HUMIDITY_LOW;
  bool HUMIDITY_HIGH;
} alarmFlags;

#define NAN_INT_0 0x0FB7
#define NAN_INT_1 0x0FB8
#define NAN_INT_2 0x0FB9
// VP starts on 4096 -> 0x1000; []

// Global variables
SensorInputAverageModel temperatureSensor(0x1001, &sensorMutex, 15);  // temperatura atual do sensor
SensorInputAverageModel humiditySensor(0x1003, &sensorMutex, 15);     // umidade atual do sensor

Uint16StorageModel temperatureSetPoint(0x1002, &variableMutex, 70, 160);  // define qual a temperatura desejada
Uint16StorageModel humiditySetPoint(0x1004, &variableMutex, 70, 140);     // define qual a umidade desejada

Uint16StorageModel temperatureFanDiffParam(NAN_INT_1, &variableMutex, 1, 20);               // diferença entre ajuste/sensor de temperatura para acionador da ventoinha
Uint16StorageModel injectionMachineDiffParam(NAN_INT_1, &variableMutex, 1, 20);             // diferença entre ajuste/sensor de temperatura para acionador da máquina injetora (depende da venoinha estar acionada)
Uint16StorageModel humidityDamperDiffParam(NAN_INT_1, &variableMutex, 1, 20);               // diferença entre ajuste/sensor de humidade para acionador do flap de umidade
Uint16StorageModel securityModeTemperatureDiffParam(NAN_INT_1, &variableMutex, 5, 100);     // diferença entre ajuste/sensor de temperatura para entrar no modo de segurança (setar valor alto para desativar)
Uint16StorageModel humidityDamperEnableTimeParam(NAN_INT_2, &variableMutex, 20, 65535);     // [MILLIS] define tempo que o flap permanece ligado para controle em estágios
Uint16StorageModel humidityDamperDisabledTimeParam(NAN_INT_0, &variableMutex, 0, 3600);     // [SECONDS] define tempo que o flap permanece desligado para controle em estágios
Uint16StorageModel temperatureFanIntervalParam(NAN_INT_0, &variableMutex, 0, 3600);         // [SECONDS] tempo de intervalo entre acionamentos da ventoinha, conta a cada inversão de estado (evita falsos acionamentos por instabilidade de temperatura)
Uint16StorageModel injectionMachineIntervalParam(NAN_INT_0, &variableMutex, 0, 7200);       // [SECONDS] tempo de intervalo entre acionamentos da injetora, conta a cada inversão de estado (evita encher a fornalha)
Uint16StorageModel injectionMachineClearTimeParam(NAN_INT_0, &variableMutex, 0, 3600);      // [SECONDS] define tempo que a injetora permanece ligada após acionameto para limpeza (rosca A)
Uint16StorageModel injectionMachineEnableTimeParam(NAN_INT_0, &variableMutex, 0, 18000);    // [SECONDS] define tempo que a injetora permanece ligada (rosca A e B)
Uint16StorageModel injectionMachineDisabledTimeParam(NAN_INT_0, &variableMutex, 0, 18000);  // [SECONDS] define tempo que a injetora permanece desligada (rosca B e/ou A)

BoolStorageModel alarmEnabled(0x1006, &variableMutex);                // controlar se alarme pode ligar (0=NÃO, 1=SIM)
BoolStorageModel temperatureFanEnabled(0x1005, &variableMutex);       // controlar se a venoinha pode ligar (0=NÃO, 1=SIM)
BoolStorageModel injectionMachineEnabled(NAN_INT_0, &variableMutex);  // controlar se a injetora pode ligar (0=NÃO, 1=SIM)

Uint16StorageModel alarmReactiveParam(0x1300, &variableMutex, 0, 600);        // [MINUTES] contagem de tempo quando alarme está desligado para religar automaticamente (0=DESATIVADO)
Uint16StorageModel alarmHumidityDiffParam(0x1301, &variableMutex, 3, 30);     // diferença de ajuste/sensor de humidade para acionar o alarme (o mesmo para alto e baixo, conforme configuração de '_alarmHumidityTypeParam')
Uint16StorageModel alarmTemperatureDiffParam(0x1302, &variableMutex, 3, 30);  // diferença de ajuste/sensor de temperatura para acionar o alarme (o mesmo para alto e baixo, conforme configuração de '_alarmTemperatureTypeParam')

Uint16StorageModel alarmTemperatureTypeParam(0x1303, &variableMutex, 0, 3);  // define tipo de acionamento do alarme de temperatura (0=NENHUM, 1=BAIXO, 2=ALTO, 3=TODOS)
Uint16StorageModel alarmHumidityTypeParam(0x1304, &variableMutex, 0, 3);     // define tipo de acionamento do alarme de humidade (0=NENHUM, 1=BAIXO, 2=ALTO, 3=TODOS)
Uint16StorageModel alarmSecurityTypeParam(0x1305, &variableMutex, 0, 3);     // define tipo de acionamento do alarme do modo de segurança (0=NENHUM, 1=BAIXO, 2=ALTO, 3=TODOS)
BoolStorageModel alarmVentilationTypeParam(0x1306, &variableMutex);          // define se gera alarme de ventilação (0=NÃO, 1=SIM)
BoolStorageModel alarmElectricalSupplyTypeParam(0x1307, &variableMutex);     // define se gera alarme de falta de energia (0=NÃO, 1=SIM)

TextStorageModel wifiSsidParam(0x4000, &variableMutex, 20);                  // define o nome da rede wifi utilizada para conexão
TextStorageModel wifiPasswordParam(0x5000, &variableMutex, 20);              // define a senha da rede wifi utilizada para conexão
TextStorageModel macAddress(0x5000, &variableMutex, 20);                     // endereço MAC do dispositivo, utilizado para conexão remota
Uint16StorageModel wifiSignalQuality(0x1305, &variableMutex, 0, 100);        // intensidade do sinal wifi do dispositivo
Uint16StorageModel connectionStatus(0x1305, &variableMutex, 0, 3);           // estado da conexão wifi/servidor do dispositivo (0=DESCONECTADO, 1=CONECTANDO, 2=SEM_INTERNET, 3=CONECTADO)
Uint16StorageModel remotePasswordParam(0x1303, &variableMutex, 1000, 9999);  // define a senha de acesso remoto ao controlador (utilizada para acessar app)

std::vector<Uint16StorageModel*> numberDisplayVariables = {
    &temperatureSetPoint,
    &humiditySetPoint,
    &alarmReactiveParam,
    &alarmHumidityDiffParam,
    &alarmTemperatureDiffParam,
    &alarmTemperatureTypeParam,
    &alarmHumidityTypeParam,
    &alarmSecurityTypeParam,
    &securityModeTemperatureDiffParam,
    &remotePasswordParam,
    &connectionStatus,
    &wifiSignalQuality,
    &temperatureFanDiffParam,
    &temperatureFanIntervalParam,
    &humidityDamperDiffParam,
    &humidityDamperEnableTimeParam,
    &humidityDamperDisabledTimeParam,
    &injectionMachineDiffParam,
    &injectionMachineClearTimeParam,
    &injectionMachineEnableTimeParam,
    &injectionMachineDisabledTimeParam,
    &injectionMachineIntervalParam,
};

std::vector<BoolStorageModel*> booleanDisplayVariables = {
    &alarmEnabled,
    &temperatureFanEnabled,
    &injectionMachineEnabled,
    &alarmVentilationTypeParam,
    &alarmElectricalSupplyTypeParam,
};

std::vector<TextStorageModel*> textDisplayVariables = {
    &wifiSsidParam,
    &wifiPasswordParam,
    &macAddress,
};

#endif