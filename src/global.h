#ifndef _SHARED_GLOBALS_
#define _SHARED_GLOBALS_

#include <Preferences.h>

#include "ADS1X15.h"
#include "model/moving-average.h"
#include "model/display-int16t.h"

ADS1115 ads(0x48);

Preferences prefs;
SemaphoreHandle_t vpMutex;

MovingAverageModel sensor_temp(15);
DisplayInt16Model readTemp(1000, &vpMutex, &prefs);

#endif