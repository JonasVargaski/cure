// Docs: https://www.dwin-global.com/uploads/T5L_DGUSII-Application-Development-Guide-20220520.pdf  on page 42

#ifndef DWIN_FRAME_H
#define DWIN_FRAME_H

#include "Arduino.h"

#define MIN_ASCII 32
#define MAX_ASCII 255

#define CMD_HEAD1 0x5A
#define CMD_HEAD2 0xA5
#define CMD_WRITE 0x82
#define CMD_READ 0x83

class DwinFrame {
  private:
  int size;
  int currentIndex;
  byte *array;

  public:
  DwinFrame(int arraySize) : size(arraySize), currentIndex(0) {
    array = new byte[size]();
  }

  ~DwinFrame() {
    delete[] array;
  }

  void clear() {
    currentIndex = 0;
    memset(array, 0, size);
  }

  // (5A A5) (0C) [83] [10 00] [04] [00 00 00 59 00 19 00 00]
  bool isValid() const {
    return (array[0] == CMD_HEAD1 && array[1] == CMD_HEAD2 && array[2] == (currentIndex - 3));
  }

  bool push(byte data) {
    if (currentIndex == 0 ^ array[0] != CMD_HEAD1) {
      array[0] = data;
      currentIndex = 1;
      return false;
    }

    if (currentIndex == 1 && data != CMD_HEAD2) {
      currentIndex = 0;
      array[0] = data;
      return false;
    }

    if (currentIndex >= size || (currentIndex > 3 && (currentIndex - 2) > array[2])) {
      clear();
      array[0] = data;
      return false;
    }

    array[currentIndex++] = data;
    return isValid();
  }

  // [5A A5] [0C] (83) [10 00] [04] [00 00 00 59 00 19 00 00]
  char getInstruction() const {
    if (array[3] == CMD_WRITE)
      return 'W';
    if (array[3] == CMD_READ)
      return 'R';
    return '-';
  }

  // [5A A5] [0C] [83] (10 00) [04] [00 00 00 59 00 19 00 00]
  uint16_t getVPAddress() const {
    return isValid() ? (uint16_t)(array[4] << 8) | array[5] : 0;
  }

  // [5A A5] [0C] [83] [10 00] (04) [00 00 00 59 00 19 00 00]
  int getDataLength() const {
    return isValid() ? array[6] : 0;
  }

  // [5A A5] [0C] [83] [10 00] [04] (00 00 00 59 00 19 00 00)
  uint16_t getWorldValue(int position = 0) const {
    if (!isValid())
      return 0;

    int maxDataLength = getDataLength();
    int rangePosition = constrain(position, 0, maxDataLength);
    int index = 7 + (rangePosition * 2);
    uint16_t value = static_cast<uint16_t>(array[7] << 8) | static_cast<uint16_t>(array[8]);
    return value;
  }

  // [5A A5] [0C] (83) [10 00] [04] (00 00 00 59 00 19 00 00)
  // [5A A5] [03] [82] (4F 4B)
  String getTextValue() const {
    String value;

    if (!isValid())
      return value;

    int startIndex = array[3] == CMD_READ ? 7 : 4;

    for (int i = startIndex; i < currentIndex; i++) {
      if (array[i] >= MIN_ASCII && array[i] < MAX_ASCII) {
        value += char(array[i]);
      }
    }

    return value;
  }

  void print() const {
    Serial.printf("[DEBUG frame%s] ", isValid() ? "" : " INVALID");
    Serial.print(F("Cmd: "));
    Serial.print(getInstruction());
    Serial.print(F(", VP: "));
    Serial.printf("0x%s (%02X%02X)", String(getVPAddress(), HEX), highByte(getVPAddress()), lowByte(getVPAddress()));

    int dataLength = getDataLength();
    Serial.print(F(", DataLength: "));
    Serial.print(dataLength);
    if (dataLength > 1) {
      for (int i = 0; i < getDataLength(); i++)
        Serial.printf(", Value[%d]: %d", i, getWorldValue(i));
    } else {
      Serial.printf(", Value: %d", getWorldValue());
    }

    Serial.print(F(", TextValue: "));
    Serial.print(getTextValue());
    Serial.print(F(", [ "));

    for (int i = 0; i < currentIndex; i++)
      Serial.printf("%02X ", array[i]);

    Serial.println("]");
  }
};

#endif  // DWIN_FRAME_H