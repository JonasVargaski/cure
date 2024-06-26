#ifndef _ARRAY_UTILS_
#define _ARRAY_UTILS_

#include <Arduino.h>

template <typename T>
bool arrayContains(T item, T* array) {
  size_t length = sizeof(array) / sizeof(array[0]);
  for (size_t i = 0; i < length; i++) {
    if (array[i] == item) return true;
  }
  return false;
}

#endif