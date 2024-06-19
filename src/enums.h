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

enum eWifiStatus {
  DISCONNECTED,
  CONNECTING,
  WITHOUT_INTERNET,
  CONNECTED
};

enum eRemoteTriggerCallback {
  OTA_UPDATE = 0,
};

#endif