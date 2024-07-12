#ifndef _SHARED_GLOBALS_
#define _SHARED_GLOBALS_

#include <Arduino.h>
#include <Preferences.h>
#include <Wire.h>

#include "Adafruit_ADS1015.h"
#include "enums.h"
#include "model/average_variable.h"
#include "model/display_variable.h"
#include "vector"

SemaphoreHandle_t variableMutex;

// Alarm flags
static bool activeAlarms[eDisplayAlarm::DISPLAY_ALARM_MAX_SIZE];

// Global variables
// VP 1000~1099
AverageVariable temperatureSensor(0x1000, &variableMutex);  // temperatura atual do sensor
AverageVariable humiditySensor(0x1001, &variableMutex);     // umidade atual do sensor

// VP 1100~1199
DisplayInt temperatureSetPoint(0x1100, &variableMutex, 70, 160);  // define qual a temperatura desejada
DisplayInt humiditySetPoint(0x1101, &variableMutex, 70, 140);     // define qual a umidade desejada

// VP 1200~1299
DisplayInt temperatureFanDiffParam(0x1200, &variableMutex, 1, 20);               // diferença entre ajuste/sensor de temperatura para acionador da ventoinha
DisplayInt injectionMachineDiffParam(0x1201, &variableMutex, 1, 20);             // diferença entre ajuste/sensor de temperatura para acionador da máquina injetora (depende da venoinha estar acionada)
DisplayInt humidityDamperDiffParam(0x1202, &variableMutex, 1, 20);               // diferença entre ajuste/sensor de humidade para acionador do flap de umidade
DisplayInt securityModeTemperatureDiffParam(0x1203, &variableMutex, 6, 100);     // diferença entre ajuste/sensor de temperatura para entrar no modo de segurança (setar valor alto para desativar)
DisplayInt humidityDamperEnableTimeParam(0x1204, &variableMutex, 80, 65535);     // [MILLIS] define tempo que o flap permanece ligado para controle em estágios
DisplayInt humidityDamperDisableTimeParam(0x1205, &variableMutex, 0, 3600);      // [SECONDS] define tempo que o flap permanece desligado para controle em estágios
DisplayInt injectionMachineClearTimeParam(0x1208, &variableMutex, 0, 3600);      // [SECONDS] define tempo que a injetora permanece ligada após acionameto para limpeza (rosca A)
DisplayInt injectionMachineEnableTimeParam(0x1209, &variableMutex, 0, 18000);    // [SECONDS] define tempo que a injetora permanece ligada (rosca A e B)
DisplayInt injectionMachineDisabledTimeParam(0x1210, &variableMutex, 0, 18000);  // [SECONDS] define tempo que a injetora permanece desligada (rosca B e/ou A)
DisplayInt alarmReactiveParam(0x1211, &variableMutex, 0, 600);                   // [MINUTES] contagem de tempo quando alarme está desligado para religar automaticamente (0=DESATIVADO)
DisplayInt alarmHumidityDiffParam(0x1212, &variableMutex, 3, 30);                // diferença de ajuste/sensor de humidade para acionar o alarme (o mesmo para alto e baixo, conforme configuração de '_alarmHumidityTypeParam')
DisplayInt alarmTemperatureDiffParam(0x1213, &variableMutex, 3, 30);             // diferença de ajuste/sensor de temperatura para acionar o alarme (o mesmo para alto e baixo, conforme configuração de '_alarmTemperatureTypeParam')
DisplayInt alarmTemperatureTypeParam(0x1214, &variableMutex, 0, 3);              // define tipo de acionamento do alarme de temperatura [eAlarmEnableType] (0=NENHUM, 1=BAIXO, 2=ALTO, 3=TODOS)
DisplayInt alarmHumidityTypeParam(0x1215, &variableMutex, 0, 3);                 // define tipo de acionamento do alarme de humidade [eAlarmEnableType] (0=NENHUM, 1=BAIXO, 2=ALTO, 3=TODOS)
DisplayInt alarmSecurityTypeParam(0x1216, &variableMutex, 0, 3);                 // define tipo de acionamento do alarme do modo de segurança [eAlarmEnableType] (0=NENHUM, 1=BAIXO, 2=ALTO, 3=TODOS)
DisplayInt alarmVentilationTypeParam(0x1217, &variableMutex, 0, 1);              // define se gera alarme de ventilação [eYesOrNo] (0=NÃO, 1=SIM)
DisplayInt alarmElectricalSupplyTypeParam(0x1218, &variableMutex, 0, 1);         // define se gera alarme de falta de energia [eYesOrNo] (0=NÃO, 1=SIM)
DisplayInt remotePasswordParam(0x1219, &variableMutex, 1, 9999);                 // define a senha de acesso remoto ao controlador (utilizada para acessar app)
DisplayInt temperatureFanReactiveParam(0x1220, &variableMutex, 0, 600);          // [MINUTES] contagem de tempo quando ventoinha está desligada para religar automaticamente (0=DESATIVADO)
DisplayInt failFlagsBlockParam(0x1221, &variableMutex, 0, 1);                    // define se habilita bloqueio/modo de segurança por falta de ventilação ou energia [eYesOrNo] (0=NÃO, 1=SIM)
DisplayInt humiditySensorType(0x1222, &variableMutex, 0, 1);                     // Tipo do sensor de umidade [eYesOrNo] (0=ºF, 1=%)
DisplayInt temperatureSensorType(0x1223, &variableMutex, 0, 1);                  // Tipo do sensor de temperatura [eYesOrNo] (0=ºF, 1=ºC)

// VP 1300~1399
DisplayInt alarmEnabled(0x1300, &variableMutex, 0, 1);             // controlar se alarme pode ligar [eYesOrNo] (0=NÃO, 1=SIM)
DisplayInt temperatureFanEnabled(0x1301, &variableMutex, 0, 1);    // controlar se a venoinha pode ligar [eYesOrNo] (0=NÃO, 1=SIM)
DisplayInt injectionMachineEnabled(0x1302, &variableMutex, 0, 1);  // controlar se a injetora pode ligar [eYesOrNo] (0=NÃO, 1=SIM)

// VP 1400~1499
DisplayText wifiSsidParam(0x1400, &variableMutex, 20);      // define o nome da rede wifi utilizada para conexão
DisplayText wifiPasswordParam(0x1425, &variableMutex, 20);  // define a senha da rede wifi utilizada para conexão
DisplayText wifiDeviceId(0x1450, &variableMutex, 14);       // endereço MAC do dispositivo, utilizado para conexão remota
DisplayText firmwareVersion(0x1465, &variableMutex, 18);    // versão do sofware do dispositivo, defines/version.FIRMWARE_VERSION

// VP 1500~1599
DisplayInt connectionStatus(0x1500, &variableMutex, 0, 3);              // estado da conexão wifi/servidor do dispositivo [eWifiStatus] (0=DESCONECTADO, 1=CONECTANDO, 2=SEM_INTERNET, 3=CONECTADO)
DisplayInt wifiSignalQuality(0x1501, &variableMutex, 0, 4);             // intensidade do sinal wifi do dispositivo (0=DESCONHECIDO 1=RUIM, 2=MEDIO, 3=BOM, 4=ÓTIMO)
DisplayInt alarmOutputState(0x1502, &variableMutex, 0, 1);              // indicador do estado da saida do alarme [eOnOff] (0=DESLIGADO, 1=LIGADO)
DisplayInt humidityDamperOutputState(0x1503, &variableMutex, 0, 2);     // indicador do estado da saida do damper de alarme [eHumidityDamperStatus](0=DESLIGADO, 1=ABRINDO, 2=FECHANDO)
DisplayInt temperatureFanOutputState(0x1504, &variableMutex, 0, 1);     // indicador do estado da saida da ventoinha [eOnOff] (0=DESLIGADO, 1=LIGADO)
DisplayInt injectionMachineOutputStateA(0x1505, &variableMutex, 0, 1);  // indicador do estado da saida 1 da injetora [eInjectionMachineStatus] (0=DESLIGADO, 1=LIGADO)
DisplayInt injectionMachineOutputStateB(0x1506, &variableMutex, 0, 1);  // indicador do estado da saida 2 da injetora [eInjectionMachineStatus] (0=DESLIGADO, 1=LIGADO)
DisplayInt tempStatusFlagIndicator(0x1507, &variableMutex, 0, 2);       // indicador de flag de temperatura (0=OK, 1=BAIXA, 2=ALTA)
DisplayInt humidityStatusFlagIndicator(0x1508, &variableMutex, 0, 2);   // indicador de flag de humidade (0=OK, 1=BAIXA, 2=ALTA)
DisplayInt energySupplyInputState(0x1509, &variableMutex, 0, 1);        // indicador de sinal de rede elétrica (0=FALHA, 1=OK)
DisplayInt remoteFailInputState(0x1510, &variableMutex, 0, 1);          // indicador de sinal de falha externa (0=FALHA, 1=OK)
DisplayInt alarmReasons(0x1511, &variableMutex, 0, 10);                 // indicador de alarme gerado (1=FALHA_SENSOR_TEMP, 2=FALHA_SENSOR_UMID, 3=SEGURANÇA_ALTA, 4=SEGURANÇA_BAIXA, 5=ALTA_TEMP, 6=BAIXA_TEMP, 7=ALTA_UMID, 8=BAIXA_UMID, 9=FALHA_ELETRICA, 10=FALHA_EXTERNA)
DisplayInt workingTimeInHours(0x1512, &variableMutex, 0, 65535);        // Tempo em horas que o controlador está ligado (desde o início)
DisplayInt skipFactoryResetFlag(0x1580, &variableMutex, 0, 1);          // reset de fábrica (0=NÂO, 1=SIM)

std::vector<DisplayInt*> displayIntObjects = {
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
    &humidityDamperDiffParam,
    &humidityDamperEnableTimeParam,
    &humidityDamperDisableTimeParam,
    &injectionMachineDiffParam,
    &injectionMachineClearTimeParam,
    &injectionMachineEnableTimeParam,
    &injectionMachineDisabledTimeParam,
    &humidityDamperOutputState,
    &temperatureFanReactiveParam,
    &tempStatusFlagIndicator,
    &humidityStatusFlagIndicator,
    &alarmReasons,
    &workingTimeInHours,
    &alarmVentilationTypeParam,
    &alarmElectricalSupplyTypeParam,
    &failFlagsBlockParam,
    &humiditySensorType,
    &temperatureSensorType,
    &alarmEnabled,
    &temperatureFanEnabled,
    &injectionMachineEnabled,
    &alarmOutputState,
    &temperatureFanOutputState,
    &injectionMachineOutputStateA,
    &injectionMachineOutputStateB,
    &energySupplyInputState,
    &remoteFailInputState,
    &skipFactoryResetFlag,
};

std::vector<DisplayText*> displayTextObjects = {
    &wifiSsidParam,
    &wifiPasswordParam,
    &wifiDeviceId,
    &firmwareVersion,
};

#endif