#ifndef _APP_UTILS_H_
#define _APP_UTILS_H_
#include "Arduino.h"

String hexToStr(const unsigned long &h, const byte &l = 8) {
  String s;
  s = String(h, HEX);
  s.toUpperCase();
  s = ("00000000" + s).substring(s.length() + 8 - l);
  return s;
}

uint32_t random32() {
  return ((uint32_t)random(0xFFFF) << 16) | random(0xFFFF);
}

String randomUUID() {
  char uuid[37];
  uint32_t a = random32();
  uint32_t b = random32();
  sprintf(uuid, "%08lX-%04lX-4%03lX-%04lX-%08lX",
          (a >> 16) & 0xFFFF, (a >> 0) & 0xFFFF,
          (b >> 12) & 0xFFF, (b >> 0) & 0xFFFF,
          (b >> 16) & 0xFFFF);
  return String(uuid);
}

#endif