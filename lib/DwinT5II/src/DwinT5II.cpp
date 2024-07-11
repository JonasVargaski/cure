#include "DwinT5II.h"

#define CMD_TIMEOUT 80
#define CMD_HEAD1 0x5A
#define CMD_HEAD2 0xA5
#define CMD_WRITE 0x82
#define CMD_READ 0x83

#define HEADER_SIZE 3

DataFrame DwinT5II::_dataFrame;
DwinT5II::hmiListener DwinT5II::_listenerCallback = nullptr;

SemaphoreHandle_t DwinT5II::_bufferMutex = nullptr;

DwinT5II::DwinT5II(HardwareSerial &uart) : _uart(&uart) {
  _bufferMutex = xSemaphoreCreateMutex();

  xTaskCreatePinnedToCore(this->backgroundProcessorTask, "DwinbackgroundProcessorTask", 2048, this, 2, &_taskHandleProcessor, 0);
}

DwinT5II::~DwinT5II() {
  if (_taskHandleProcessor) vTaskDelete(_taskHandleProcessor);
}

void DwinT5II::setCallback(hmiListener callBackFunction) {
  _listenerCallback = callBackFunction;
}

FrameState DwinT5II::sendCommand(const uint8_t *command, const uint8_t &cmdLength) {
  if (xSemaphoreTake(_bufferMutex, pdMS_TO_TICKS(300)) == pdTRUE) {
    processBuffer();
    _uart->write(command, cmdLength);
    readSerialToBuffer();
    xSemaphoreGive(_bufferMutex);
    return getNextFrame();
  } else {
    Serial.println(F("[HMI] sendCommand timeout."));
    return FrameState::INVALID;
  }
}

void DwinT5II::setBrightness(const uint8_t brightness) {
  uint8_t brtn = constrain(brightness, 0, 127);
  byte sendBuffer[] = {CMD_HEAD1, CMD_HEAD2, 0x04, CMD_WRITE, 0x00, CMD_WRITE, brtn};
  sendCommand(sendBuffer, sizeof(sendBuffer));
}

void DwinT5II::setPage(const uint8_t &page) {
  byte sendBuffer[] = {CMD_HEAD1, CMD_HEAD2, 0x07, CMD_WRITE, 0x00, 0x84, 0x5A, 0x01, 0x00, page};
  sendCommand(sendBuffer, sizeof(sendBuffer));
}

int DwinT5II::getPage() {
  byte sendBuffer[] = {CMD_HEAD1, CMD_HEAD2, 0x04, CMD_READ, 0x00, 0x14, 0x01};
  DataFrame *frame = getResponseFrame(sendCommand(sendBuffer, sizeof(sendBuffer)), 0x0014);
  if (frame != nullptr) return frame->getInt();
  return -1;
}

void DwinT5II::setVP(uint16_t address, int data) {
  byte sendBuffer[] = {CMD_HEAD1, CMD_HEAD2, 0x05, CMD_WRITE, (uint8_t)((address >> 8) & 0xFF), (uint8_t)((address) & 0xFF), (uint8_t)((data >> 8) & 0xFF), (uint8_t)((data) & 0xFF)};
  FrameState result = sendCommand(sendBuffer, sizeof(sendBuffer));
  if (result == FrameState::COMPLETE) {
    if (_dataFrame.isOkResponse()) return;
    if (_listenerCallback != nullptr) _listenerCallback(&_dataFrame);
  }
}

void DwinT5II::setVP(uint16_t address, float data) {
  byte hx[4] = {0};
  byte *new_bytes = reinterpret_cast<byte *>(&data);
  memcpy(hx, new_bytes, 4);
  byte sendBuffer[] = {CMD_HEAD1, CMD_HEAD2, 0x07, CMD_WRITE, (uint8_t)((address >> 8) & 0xFF), (uint8_t)((address) & 0xFF), hx[3], hx[2], hx[1], hx[0]};
  FrameState result = sendCommand(sendBuffer, sizeof(sendBuffer));
  if (result == FrameState::COMPLETE) {
    if (_dataFrame.isOkResponse()) return;
    if (_listenerCallback != nullptr) _listenerCallback(&_dataFrame);
  }
}

void DwinT5II::setVP(uint16_t address, const String &textData) {
  byte sendBuffer[textData.length() + 7];
  sendBuffer[0] = CMD_HEAD1;
  sendBuffer[1] = CMD_HEAD2;
  sendBuffer[2] = textData.length() + 4;
  sendBuffer[3] = CMD_WRITE;
  sendBuffer[4] = highByte(address);
  sendBuffer[5] = lowByte(address);
  textData.getBytes(sendBuffer + 6, textData.length() + 1);
  FrameState result = sendCommand(sendBuffer, sizeof(sendBuffer));
  if (result == FrameState::COMPLETE) {
    if (_dataFrame.isOkResponse()) return;
    if (_listenerCallback != nullptr) _listenerCallback(&_dataFrame);
  }
}

DataFrame *DwinT5II::readVP(uint16_t address, byte countAddress) {
  byte sendBuffer[] = {CMD_HEAD1, CMD_HEAD2, 0x04, CMD_READ, (uint8_t)((address >> 8) & 0xFF), (uint8_t)((address) & 0xFF), countAddress};
  return getResponseFrame(sendCommand(sendBuffer, sizeof(sendBuffer)), address);
}

DataFrame *DwinT5II::getResponseFrame(FrameState result, const uint16_t address) {
  if (result == FrameState::COMPLETE) {
    int retryCount = 0;
    while (retryCount++ < 3) {
      if (_dataFrame.getAddress() == address) return &_dataFrame;
      if (_listenerCallback != nullptr) _listenerCallback(&_dataFrame);
      getNextFrame();
    }
  }
  return nullptr;
}

void DwinT5II::backgroundProcessorTask(void *parameter) {
  DwinT5II *p_dwin = static_cast<DwinT5II *>(parameter);

  while (true) {
    if (xSemaphoreTake(_bufferMutex, pdMS_TO_TICKS(CMD_TIMEOUT)) == pdTRUE) {
      p_dwin->readSerialToBuffer();
      p_dwin->processBuffer();
      xSemaphoreGive(p_dwin->_bufferMutex);
    }
    vTaskDelay(pdMS_TO_TICKS(5));
  }
}

void DwinT5II::readSerialToBuffer() {
  if (_bufferIndex >= BUFFER_SIZE) {
    Serial.println("[HMI] Buffer overflow");
    return;
  }

  TickType_t xLastWakeTime = xTaskGetTickCount();
  TickType_t xEndTime = xLastWakeTime + pdMS_TO_TICKS(CMD_TIMEOUT);
  bool receivedData = false;

  while (xTaskGetTickCount() < xEndTime) {
    if (_uart->available() > 0) {
      while (_uart->available() > 0 && (_bufferIndex + 1) <= BUFFER_SIZE) {
        _buffer[_bufferIndex] = (byte)_uart->read();
        _bufferIndex++;
      }
      return;
    }
    vTaskDelay(pdMS_TO_TICKS(1));
  }
}

void DwinT5II::processBuffer() {
  while (_bufferIndex >= HEADER_SIZE) {
    FrameState result = getNextFrame();
    if (result == FrameState::INVALID) {
      memmove(_buffer, _buffer + 1, _bufferIndex - 1);
      _bufferIndex--;
    } else if (result == FrameState::WAITING) {
      break;
    } else if (_listenerCallback != nullptr) {
      _listenerCallback(&_dataFrame);
    }
  }
}

FrameState DwinT5II::getNextFrame() {
  if (_buffer[0] == CMD_HEAD1 && _buffer[1] == CMD_HEAD2 && (_buffer[3] == CMD_WRITE || _buffer[3] == CMD_READ)) {
    int contentLength = _buffer[2];
    if (_bufferIndex >= contentLength + HEADER_SIZE) {
      _dataFrame.clear();
      _dataFrame.setData(_buffer + HEADER_SIZE, contentLength);
      memmove(_buffer, _buffer + contentLength + HEADER_SIZE, _bufferIndex - (contentLength + HEADER_SIZE));
      _bufferIndex -= (contentLength + HEADER_SIZE);
      return FrameState::COMPLETE;
    } else {
      return FrameState::WAITING;
    }
  }
  return FrameState::INVALID;
}
