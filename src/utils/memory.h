#ifndef _MEMORY_UTILS_
#define _MEMORY_UTILS_

#include <Arduino.h>

float getMemoryUsedPercentage() {
  size_t heapTotal = ESP.getHeapSize();
  size_t heapFree = esp_get_free_heap_size();
  size_t heapUsed = heapTotal - heapFree;
  float usedPercentage = (static_cast<float>(heapUsed) / heapTotal) * 100.0;
  return usedPercentage;
}

#endif