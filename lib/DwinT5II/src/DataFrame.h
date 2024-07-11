
// docs: https://www.dwin-global.com/uploads/T5L_DGUSII-Application-Development-Guide-20220520.pdf
#ifndef DataFrame_h
#define DataFrame_h

#include "Arduino.h"
#define DATA_FRAME_MAX_LENGTH 36

class DataFrame {
  private:
  size_t _length;
  byte _data[DATA_FRAME_MAX_LENGTH];

  public:
  DataFrame() : _length(0) {
    memset(_data, 0, DATA_FRAME_MAX_LENGTH);
  }

  void setData(byte* inputData, size_t len) {
    clear();
    if (len > DATA_FRAME_MAX_LENGTH) len = DATA_FRAME_MAX_LENGTH;
    _length = len;
    memcpy(_data, inputData, _length);
  }

  void clear() {
    memset(_data, 0, _length);
    _length = 0;
  }

  void print() {
    Serial.print(F("[HMI] "));
    for (size_t i = 0; i < _length; ++i) {
      Serial.printf("%02X ", _data[i]);
    }
    Serial.printf("| vp: %02X%02X | int: %d | ascii: %s |\n", highByte(getAddress()), lowByte(getAddress()), getInt(), getAscii().c_str());
  }

  byte* data() {
    return _data;
  }

  size_t length() {
    return _length;
  }

  bool isOkResponse() {
    if (_data[0] == 0x82 && _data[1] == 0x4F && _data[2] == 0x4B) return true;
    return false;
  }

  uint16_t getAddress() {
    if (_data[0] != 0x83 || _length < 4) return 0;
    return (uint16_t)(_data[1] << 8) | _data[2];
  }

  // returns number of addresses. e.g., requesting VP from 1000 to 1010 returns 10.
  int getAddressCount() {
    if (_data[0] != 0x83 || _length < 3) return 0;
    return static_cast<int>(_data[3]);
  }

  String getAscii() {
    String asciiStr = "";
    if (_data[0] == 0x83) {
      for (int i = 4; i < _length; i++) {
        if ((_data[i] == 0xFF) && (_data[i + 1] == 0xFF)) break;
        asciiStr += static_cast<char>(_data[i]);
      }
    }
    return asciiStr;
  }

  int getInt(int addressPosition = 0) {
    int count = getAddressCount();
    if (!count || addressPosition > count) return -1;

    int num = 0;
    int nextIndex = addressPosition * 2;
    num = static_cast<uint16_t>(_data[4 + nextIndex] << 8) | static_cast<uint16_t>(_data[5 + nextIndex]);
    return num;
  }
};

#endif

// set VP response
// payload  5A A5 03 82 4F 4B
// [FRAME]  82 4F 4B          === OK command

// read VP response
// payload  5A A5 06 83 11 01 01 00 03
// [FRAME]  83 11 01 01 00 03

// screen change VP WORD
// payload  5A A5 06 83 11 01 01 00 07
// [FRAME]  83 11 01 01 00 07

// screen change VP TEXT
// payload  5A A5 0A 83 14 00 03 6A 6F 6E FF FF 00
// [FRAME] 83 14 00 03 6A 6F 6E FF FF 00