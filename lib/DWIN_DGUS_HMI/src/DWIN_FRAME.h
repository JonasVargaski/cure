#ifndef DWIN_FRAME_H
#define DWIN_FRAME_H

#include "Arduino.h"

class DwinFrame
{
private:
  int size;
  int currentIndex;

public:
  byte *array;
  DwinFrame(int arraySize)
  {
    size = arraySize;
    currentIndex = 0;
    array = new byte[size];
    for (int i = 0; i < size; i++)
    {
      array[i] = 0;
    }
  }

  ~DwinFrame()
  {
    delete[] array;
  }

  bool isValid()
  {
    return (array[0] == 0x5A && array[1] == 0xA5 && array[2] == (currentIndex - 3));
  }

  bool push(byte data)
  {
    if (currentIndex >= size)
      return false;

    if (array[0] != 0x5A)
    {
      array[0] = data;
      currentIndex = 1;
      return false;
    }

    if (currentIndex == 1 && data != 0xA5)
    {
      currentIndex = 0;
      array[0] = data;
      return false;
    }

    array[currentIndex] = data;
    currentIndex++;
    return isValid();
  }

  char getInstruction()
  {
    if (array[3] == 0x82)
      return 'W';
    if (array[3] == 0x83)
      return 'R';
    return '-';
  }

  uint16_t getVPAddress()
  {
    return isValid() ? (uint16_t)(array[4] << 8) | array[5] : 0;
  }

  uint16_t getWorldValue(int address = 0)
  {
    if (!isValid() || address < 0 || address + 1 >= size)
      return 0;

    int index = address + 4;
    return (uint16_t)(array[index] << 8) | array[index + 1];
  }

  String getTextValue()
  {
    String value = "";

    if (!isValid())
      return value;

    for (int i = 4; i < currentIndex; i++)
      value.concat(char(array[i]));

    return value;
  }

  void print()
  {
    Serial.print(F("[Frame] valid: "));
    Serial.print(isValid() ? "true" : "false");
    Serial.print(F(", Instruction: "));
    Serial.print(getInstruction());
    Serial.print(F(", VP: "));
    Serial.print(getVPAddress());
    Serial.print(F(", WorldValue: "));
    Serial.print(getWorldValue());
    Serial.print(F(", TextValue: "));
    Serial.print(getTextValue());
    Serial.print(F("  |  "));

    for (int i = 0; i < currentIndex; i++)
    {
      Serial.printf("%02X ", array[i]);
    }
    Serial.println();
  }
};

#endif // DWIN_FRAME_H