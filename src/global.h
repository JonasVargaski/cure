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

// Global variables
// VP 1000~1099
SensorInputAverageModel temperatureSensor(0x1000, &sensorMutex, 15);  // temperatura atual do sensor
SensorInputAverageModel humiditySensor(0x1001, &sensorMutex, 15);     // umidade atual do sensor

// VP 1100~1199
Uint16StorageModel temperatureSetPoint(0x1100, &variableMutex, 70, 160);  // define qual a temperatura desejada
Uint16StorageModel humiditySetPoint(0x1101, &variableMutex, 70, 140);     // define qual a umidade desejada

// VP 1200~1299
Uint16StorageModel temperatureFanDiffParam(0x1200, &variableMutex, 1, 20);               // diferença entre ajuste/sensor de temperatura para acionador da ventoinha
Uint16StorageModel injectionMachineDiffParam(0x1201, &variableMutex, 1, 20);             // diferença entre ajuste/sensor de temperatura para acionador da máquina injetora (depende da venoinha estar acionada)
Uint16StorageModel humidityDamperDiffParam(0x1202, &variableMutex, 1, 20);               // diferença entre ajuste/sensor de humidade para acionador do flap de umidade
Uint16StorageModel securityModeTemperatureDiffParam(0x1203, &variableMutex, 5, 100);     // diferença entre ajuste/sensor de temperatura para entrar no modo de segurança (setar valor alto para desativar)
Uint16StorageModel humidityDamperEnableTimeParam(0x1204, &variableMutex, 80, 65535);     // [MILLIS] define tempo que o flap permanece ligado para controle em estágios
Uint16StorageModel humidityDamperDisableTimeParam(0x1205, &variableMutex, 0, 3600);      // [SECONDS] define tempo que o flap permanece desligado para controle em estágios
Uint16StorageModel temperatureFanIntervalParam(0x1206, &variableMutex, 0, 3600);         // [SECONDS] tempo de intervalo entre acionamentos da ventoinha, conta a cada inversão de estado (evita falsos acionamentos por instabilidade de temperatura)
Uint16StorageModel injectionMachineIntervalParam(0x1207, &variableMutex, 0, 7200);       // [SECONDS] tempo de intervalo entre acionamentos da injetora, conta a cada inversão de estado (evita encher a fornalha)
Uint16StorageModel injectionMachineClearTimeParam(0x1208, &variableMutex, 0, 3600);      // [SECONDS] define tempo que a injetora permanece ligada após acionameto para limpeza (rosca A)
Uint16StorageModel injectionMachineEnableTimeParam(0x1209, &variableMutex, 0, 18000);    // [SECONDS] define tempo que a injetora permanece ligada (rosca A e B)
Uint16StorageModel injectionMachineDisabledTimeParam(0x1210, &variableMutex, 0, 18000);  // [SECONDS] define tempo que a injetora permanece desligada (rosca B e/ou A)
Uint16StorageModel alarmReactiveParam(0x1211, &variableMutex, 0, 600);                   // [MINUTES] contagem de tempo quando alarme está desligado para religar automaticamente (0=DESATIVADO)
Uint16StorageModel alarmHumidityDiffParam(0x1212, &variableMutex, 3, 30);                // diferença de ajuste/sensor de humidade para acionar o alarme (o mesmo para alto e baixo, conforme configuração de '_alarmHumidityTypeParam')
Uint16StorageModel alarmTemperatureDiffParam(0x1213, &variableMutex, 3, 30);             // diferença de ajuste/sensor de temperatura para acionar o alarme (o mesmo para alto e baixo, conforme configuração de '_alarmTemperatureTypeParam')
Uint16StorageModel alarmTemperatureTypeParam(0x1214, &variableMutex, 0, 3);              // define tipo de acionamento do alarme de temperatura [eAlarmType] (0=NENHUM, 1=BAIXO, 2=ALTO, 3=TODOS)
Uint16StorageModel alarmHumidityTypeParam(0x1215, &variableMutex, 0, 3);                 // define tipo de acionamento do alarme de humidade [eAlarmType] (0=NENHUM, 1=BAIXO, 2=ALTO, 3=TODOS)
Uint16StorageModel alarmSecurityTypeParam(0x1216, &variableMutex, 0, 3);                 // define tipo de acionamento do alarme do modo de segurança [eAlarmType] (0=NENHUM, 1=BAIXO, 2=ALTO, 3=TODOS)
BoolStorageModel alarmVentilationTypeParam(0x1217, &variableMutex);                      // define se gera alarme de ventilação [eYesOrNo] (0=NÃO, 1=SIM)
BoolStorageModel alarmElectricalSupplyTypeParam(0x1218, &variableMutex);                 // define se gera alarme de falta de energia [eYesOrNo] (0=NÃO, 1=SIM)
Uint16StorageModel remotePasswordParam(0x1219, &variableMutex, 1000, 9999);              // define a senha de acesso remoto ao controlador (utilizada para acessar app)
Uint16StorageModel temperatureFanReactiveParam(0x1220, &variableMutex, 0, 600);          // [MINUTES] contagem de tempo quando ventoinha está desligada para religar automaticamente (0=DESATIVADO)
BoolStorageModel failFlagsBlockParam(0x1221, &variableMutex);                            // define se habilita bloqueio/modo de segurança por falta de ventilação ou energia [eYesOrNo] (0=NÃO, 1=SIM)

// VP 1300~1399
BoolStorageModel alarmEnabled(0x1300, &variableMutex);             // controlar se alarme pode ligar [eYesOrNo] (0=NÃO, 1=SIM)
BoolStorageModel temperatureFanEnabled(0x1301, &variableMutex);    // controlar se a venoinha pode ligar [eYesOrNo] (0=NÃO, 1=SIM)
BoolStorageModel injectionMachineEnabled(0x1302, &variableMutex);  // controlar se a injetora pode ligar [eYesOrNo] (0=NÃO, 1=SIM)

// VP 1400~1499
TextStorageModel wifiSsidParam(0x1400, &variableMutex, 20);      // define o nome da rede wifi utilizada para conexão
TextStorageModel wifiPasswordParam(0x1425, &variableMutex, 20);  // define a senha da rede wifi utilizada para conexão
TextStorageModel wifiDeviceId(0x1450, &variableMutex, 8);        // endereço MAC do dispositivo, utilizado para conexão remota

// VP 1500~1599
Uint16StorageModel connectionStatus(0x1500, &variableMutex, 0, 3);             // estado da conexão wifi/servidor do dispositivo [eWifiStatus] (0=DESCONECTADO, 1=CONECTANDO, 2=SEM_INTERNET, 3=CONECTADO)
Uint16StorageModel wifiSignalQuality(0x1501, &variableMutex, 0, 4);            // intensidade do sinal wifi do dispositivo (0=DESCONHECIDO 1=RUIM, 2=MEDIO, 3=BOM, 4=ÓTIMO)
BoolStorageModel alarmOutputState(0x1502, &variableMutex);                     // indicador do estado da saida do alarme [eOnOff] (0=DESLIGADO, 1=LIGADO)
Uint16StorageModel humidityDamperOutputState(0x1503, &variableMutex, 0, 2);    // indicador do estado da saida do damper de alarme [eHumidityDamperStatus](0=DESLIGADO, 1=ABRINDO, 2=FECHANDO)
BoolStorageModel temperatureFanOutputState(0x1504, &variableMutex);            // indicador do estado da saida da ventoinha [eOnOff] (0=DESLIGADO, 1=LIGADO)
Uint16StorageModel injectionMachineOutputState(0x1505, &variableMutex, 0, 2);  // indicador do estado da saida da injetora [eInjectionMachineStatus] (0=DESLIGADO, 1=LIGADO, 2=EM LIMPEZA)

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
    &humidityDamperDisableTimeParam,
    &injectionMachineDiffParam,
    &injectionMachineClearTimeParam,
    &injectionMachineEnableTimeParam,
    &injectionMachineDisabledTimeParam,
    &injectionMachineIntervalParam,
    &humidityDamperOutputState,
    &injectionMachineOutputState,
    &temperatureFanReactiveParam,
};

std::vector<BoolStorageModel*> booleanDisplayVariables = {
    &alarmEnabled,
    &temperatureFanEnabled,
    &injectionMachineEnabled,
    &alarmVentilationTypeParam,
    &alarmElectricalSupplyTypeParam,
    &alarmOutputState,
    &temperatureFanOutputState,
    &failFlagsBlockParam,
};

std::vector<TextStorageModel*> textDisplayVariables = {
    &wifiSsidParam,
    &wifiPasswordParam,
    &wifiDeviceId,
};

#endif