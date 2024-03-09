#ifndef _SHARED_GLOBALS_
#define _SHARED_GLOBALS_

#include <Preferences.h>

#include "model/moving-average.h"
#include "model/display-int16t.h"

#define pinCount 8

Preferences prefs;
SemaphoreHandle_t vpMutex;

MovingAverageModel sensor_temp(5);
DisplayInt16Model readTemp(1000, &vpMutex, &prefs);

// Pin map
int pins[pinCount] = {13, 12, 14, 27, 26, 25, 33, 32};

#endif