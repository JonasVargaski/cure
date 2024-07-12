#ifndef DwinT5II_h
#define DwinT5II_h

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>

#include "Arduino.h"
#include "DataFrame.h"

#define BUFFER_SIZE 64

enum class FrameState {
  WAITING,
  INVALID,
  COMPLETE
};

class DwinT5II {
  public:
  DwinT5II(HardwareSerial &uart);
  ~DwinT5II();

  typedef void (*hmiListener)(DataFrame *frame);
  static void setCallback(hmiListener callBackFunction);

  void setVP(uint16_t address, int data);
  void setVP(uint16_t address, float data);
  void setVP(uint16_t address, const String &data);
  DataFrame *readVP(uint16_t address, byte countAddress);
  void setBrightness(const uint8_t brightness);
  void setPage(const uint8_t &pageNum);
  void beep(int durationInMs);
  int getPage();

  private:
  HardwareSerial *_uart;
  byte _buffer[BUFFER_SIZE];
  volatile int _bufferIndex = 0;
  static hmiListener _listenerCallback;
  static DataFrame _dataFrame;
  static SemaphoreHandle_t _bufferMutex;

  FrameState sendCommand(const uint8_t *command, const uint8_t &cmdLength);
  DataFrame *getResponseFrame(FrameState result, const uint16_t address);

  protected:
  void readSerialToBuffer(void);
  void processBuffer();
  FrameState getNextFrame(void);

  static void backgroundProcessorTask(void *parameter);
  TaskHandle_t _taskHandleProcessor = nullptr;
};

#endif  // DwinT5II_h