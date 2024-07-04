#ifndef _ENUMS_H_
#define _ENUMS_H_

enum eAlarmEnableType {
  NONE,
  LOW_VALUE,
  HIGH_VALUE,
  ALL
};

enum eHumidityDamperStatus {
  DAMPER_OFF,
  DAMPER_OPEN,
  DAMPER_CLOSE
};

enum eInjectionMachineStatus {
  MACHINE_OFF,
  MACHINE_ON,
  MACHINE_CLEAR
};

enum eYesOrNo {
  YESNO_NO,
  YESNO_YES
};

enum eOnOff {
  ONOFF_OFF,
  ONOFF_ON
};

enum eArrowIcon {
  ARROW_NONE,
  ARROW_DOWN,
  ARROW_UP
};

enum eWifiStatus {
  DISCONNECTED,
  CONNECTING,
  WITHOUT_INTERNET,
  CONNECTED
};

enum eRemoteTriggerCallback {
  OTA_UPDATE = 0,
};

enum eDisplayAlarm {
  ALARM_NONE,
  TEMPERATURE_SENSOR_FAIL,
  HUMIDITY_SENSOR_FAIL,
  SECURITY_MODE_HIGH,
  SECURITY_MODE_LOW,
  TEMPERATURE_HIGH,
  TEMPERATURE_LOW,
  HUMIDITY_HIGH,
  HUMIDITY_LOW,
  ELECTRICAL_FAIL,
  REMOTE_FAIL,
  DISPLAY_ALARM_MAX_SIZE
};

#endif