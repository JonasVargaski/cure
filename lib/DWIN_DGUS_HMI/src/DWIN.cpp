#include "DWIN.h"
#include <stdio.h>

#define CMD_HEAD1 0x5A
#define CMD_HEAD2 0xA5
#define CMD_WRITE 0x82
#define CMD_READ 0x83

#define MIN_ASCII 32
#define MAX_ASCII 255

#define CMD_READ_TIMEOUT 50
#define READ_TIMEOUT 50

DWIN::DWIN(HardwareSerial &port, uint8_t receivePin, uint8_t transmitPin, long baud)
{
    port.begin(baud, SERIAL_8N1, receivePin, transmitPin);
    this->_dwinSerial = (Stream *)&port;
}

// Get Hardware Firmware Version of DWIN HMI
double DWIN::getHWVersion()
{ //  HEX(5A A5 04 83 00 0F 01)
    byte sendBuffer[] = {CMD_HEAD1, CMD_HEAD2, 0x04, CMD_READ, 0x00, 0x0F, 0x01};
    _dwinSerial->write(sendBuffer, sizeof(sendBuffer));
    vTaskDelay(pdMS_TO_TICKS(10));
    return readCMDLastByte();
}

double DWIN::getGUISoftVersion()
{ //  HEX(5A A5 04 83 00 0F 01)
    byte sendBuffer[] = {CMD_HEAD1, CMD_HEAD2, 0x04, CMD_READ, 0x00, 0x0F, 0x01};
    _dwinSerial->write(sendBuffer, sizeof(sendBuffer));
    vTaskDelay(pdMS_TO_TICKS(10));
    return readCMDLastByte(1);
}

// Restart DWIN HMI
void DWIN::restartHMI()
{ // HEX(5A A5 07 82 00 04 55 aa 5a a5 )
    byte sendBuffer[] = {CMD_HEAD1, CMD_HEAD2, 0x07, CMD_WRITE, 0x00, 0x04, 0x55, 0xaa, CMD_HEAD1, CMD_HEAD2};
    _dwinSerial->write(sendBuffer, sizeof(sendBuffer));
    vTaskDelay(pdMS_TO_TICKS(10));
    readDWIN();
}

// SET DWIN Brightness
void DWIN::setBrightness(byte brightness)
{
    byte sendBuffer[] = {CMD_HEAD1, CMD_HEAD2, 0x04, CMD_WRITE, 0x00, 0x82, brightness};
    _dwinSerial->write(sendBuffer, sizeof(sendBuffer));
    readDWIN();
}

// GET DWIN Brightness
byte DWIN::getBrightness()
{
    byte sendBuffer[] = {CMD_HEAD1, CMD_HEAD2, 0x04, CMD_READ, 0x00, 0x31, 0x01};
    _dwinSerial->write(sendBuffer, sizeof(sendBuffer));
    return readCMDLastByte();
}

// Change Page
void DWIN::setPage(byte page)
{
    // 5A A5 07 82 00 84 5a 01 00 02
    byte sendBuffer[] = {CMD_HEAD1, CMD_HEAD2, 0x07, CMD_WRITE, 0x00, 0x84, 0x5A, 0x01, 0x00, page};
    _dwinSerial->write(sendBuffer, sizeof(sendBuffer));
    readDWIN();
}
// Play a sound
void DWIN::playSound(byte soundID)
{
    // 5A A5 07 82 00 A0 soundID 01 40 00
    byte sendBuffer[] = {CMD_HEAD1, CMD_HEAD2, 0x07, CMD_WRITE, 0x00, 0xA0, soundID, 0x01, 0x40, 0x00};
    _dwinSerial->write(sendBuffer, sizeof(sendBuffer));
    readDWIN();
}

// Get Current Page ID
byte DWIN::getPage()
{
    byte sendBuffer[] = {CMD_HEAD1, CMD_HEAD2, 0x04, CMD_READ, 0x00, 0x14, 0x01};
    _dwinSerial->write(sendBuffer, sizeof(sendBuffer));
    return readCMDLastByte();
}
// set the hardware RTC The first two digits of the year are automatically added
void DWIN::setRTC(byte year, byte month, byte day, byte hour, byte minute, byte second)
{
    // 5A A5 0B 82 00 9C 5A A5 year month day hour minute second
    byte sendBuffer[] = {CMD_HEAD1, CMD_HEAD2, 0x0B, CMD_WRITE, 0x00, 0x9C, 0x5A, 0xA5, year, month, day, hour, minute, second};
    _dwinSerial->write(sendBuffer, sizeof(sendBuffer));
    readDWIN();
}
// update the software RTC The first two digits of the year are automatically added
void DWIN::setRTCSOFT(byte year, byte month, byte day, byte weekday, byte hour, byte minute, byte second)
{
    // 5A A5 0B 82 0010 year month day weekday(0-6 0=Sunday) hour minute second 00
    byte sendBuffer[] = {CMD_HEAD1, CMD_HEAD2, 0x0B, CMD_WRITE, 0x00, 0x10, year, month, day, weekday, hour, minute, second, 0x00};
    _dwinSerial->write(sendBuffer, sizeof(sendBuffer));
    readDWIN();
}
// Set Text on VP Address
void DWIN::setText(long address, String textData)
{
    byte ffEnding[2] = {0xFF, 0xFF};
    int dataLen = textData.length();
    byte startCMD[] = {CMD_HEAD1, CMD_HEAD2, (uint8_t)(dataLen + 5), CMD_WRITE,
                       (uint8_t)((address >> 8) & 0xFF), (uint8_t)((address) & 0xFF)};
    byte dataCMD[dataLen];
    textData.getBytes(dataCMD, dataLen + 1);
    byte sendBuffer[8 + dataLen];

    memcpy(sendBuffer, startCMD, sizeof(startCMD));
    memcpy(sendBuffer + 6, dataCMD, sizeof(dataCMD));
    memcpy(sendBuffer + (6 + sizeof(dataCMD)), ffEnding, 2); // add ending 0xFFFF
    _dwinSerial->write(sendBuffer, sizeof(sendBuffer));
    readDWIN();
}

// Set Byte Data on VP Address makes more sense alias of below
void DWIN::setVPByte(long address, byte data)
{
    setVP(address, data);
}

void DWIN::setVP(long address, byte data)
{
    // 0x5A, 0xA5, 0x05, 0x82, 0x40, 0x20, 0x00, state
    byte sendBuffer[] = {CMD_HEAD1, CMD_HEAD2, 0x05, CMD_WRITE, (uint8_t)((address >> 8) & 0xFF), (uint8_t)((address) & 0xFF), 0x00, data};
    _dwinSerial->write(sendBuffer, sizeof(sendBuffer));
    readDWIN();
}

// Set WordData on VP Address
void DWIN::setVPWord(long address, int data)
{
    // 0x5A, 0xA5, 0x05, 0x82, hiVPaddress, loVPaddress, hiData, loData
    byte sendBuffer[] = {CMD_HEAD1, CMD_HEAD2, 0x05, CMD_WRITE, (uint8_t)((address >> 8) & 0xFF), (uint8_t)((address) & 0xFF), (uint8_t)((data >> 8) & 0xFF), (uint8_t)((data) & 0xFF)};
    _dwinSerial->write(sendBuffer, sizeof(sendBuffer));
    readDWIN();
}

// read WordData from VP Address you can read sequential multiple words returned in rx event
void DWIN::readVPWord(long address, byte numWords)
{
    // 0x5A, 0xA5, 0x04, 0x83, hiVPaddress, loVPaddress, 0x01 (1 vp to read)
    byte sendBuffer[] = {CMD_HEAD1, CMD_HEAD2, 0x04, CMD_READ, (uint8_t)((address >> 8) & 0xFF), (uint8_t)((address) & 0xFF), numWords};
    _dwinSerial->write(sendBuffer, sizeof(sendBuffer));
}

// read byte from VP Address (if hiByte = true read HiByte of word)
byte DWIN::readVPByte(long address, bool hiByte)
{
    // 0x5A, 0xA5, 0x04, 0x83, hiVPaddress, loVPaddress, 0x01)
    byte sendBuffer[] = {CMD_HEAD1, CMD_HEAD2, 0x04, CMD_READ, (uint8_t)((address >> 8) & 0xFF), (uint8_t)((address) & 0xFF), 0x1};
    _dwinSerial->write(sendBuffer, sizeof(sendBuffer));
    return readCMDLastByte(hiByte);
}

// read or write the NOR from/to VP must be on a even address 2 words are written or read
void DWIN::norReadWrite(bool write, long VPAddress, long NORAddress)
{
    byte readWrite;
    (write) ? (readWrite = 0xa5) : (readWrite = 0x5a);
    byte sendBuffer[] = {CMD_HEAD1, CMD_HEAD2, 0x0B, CMD_WRITE, 0x00, 0x08, readWrite, (uint8_t)((NORAddress >> 16) & 0xFF), (uint8_t)((NORAddress >> 8) & 0xFF),
                         (uint8_t)((NORAddress) & 0xFF), (uint8_t)((VPAddress >> 8) & 0xFF), (uint8_t)((VPAddress) & 0xFF), 0x00, 0x02};
    _dwinSerial->write(sendBuffer, sizeof(sendBuffer));
    readDWIN();
    vTaskDelay(pdMS_TO_TICKS(30)); // DWIN Docs say - appropriate delay - is this it?
}
// beep Buzzer for 1 Sec
void DWIN::beepHMI()
{
    // 0x5A, 0xA5, 0x05, 0x82, 0x00, 0xA0, 0x00, 0x7D
    byte sendBuffer[] = {CMD_HEAD1, CMD_HEAD2, 0x05, CMD_WRITE, 0x00, 0xA0, 0x00, 0x7D};
    _dwinSerial->write(sendBuffer, sizeof(sendBuffer));
    readDWIN();
}
// set text color (16-bit RGB) on controls which allow it ie. text control.
// changes the control sp address space (sp=description pointer) content see the DWIN docs.
void DWIN::setTextColor(long spAddress, long spOffset, long color)
{
    // 0x5A, 0xA5, 0x05, hi spAddress, lo spAddress, hi color, lo color
    spAddress = (spAddress + spOffset);
    byte sendBuffer[] = {CMD_HEAD1, CMD_HEAD2, 0x05, CMD_WRITE, (uint8_t)((spAddress >> 8) & 0xFF), (uint8_t)((spAddress) & 0xFF), (uint8_t)((color >> 8) & 0xFF), (uint8_t)((color) & 0xFF)};
    _dwinSerial->write(sendBuffer, sizeof(sendBuffer));
    readDWIN();
}

// set float value to 32bit DATA Variable
void DWIN::setFloatValue(long vpAddress, float fValue)
{
    byte hx[4] = {0};
    byte *new_bytes = reinterpret_cast<byte *>(&fValue);
    memcpy(hx, new_bytes, 4);
    byte sendBuffer[] = {CMD_HEAD1, CMD_HEAD2, 0x07, CMD_WRITE, (uint8_t)((vpAddress >> 8) & 0xFF), (uint8_t)((vpAddress) & 0xFF), hx[3], hx[2], hx[1], hx[0]};
    _dwinSerial->write(sendBuffer, sizeof(sendBuffer));
    readDWIN();
}

// Send array to the display we dont need the 5A A5 or
// the size byte hopefully we can work this out.
void DWIN::sendArray(byte dwinSendArray[], byte arraySize)
{
    byte sendBuffer[3 + arraySize] = {CMD_HEAD1, CMD_HEAD2, arraySize};

    memcpy(sendBuffer + 3, dwinSendArray, arraySize);
    _dwinSerial->write(sendBuffer, sizeof(sendBuffer));

    // look for the ack. on write
    if (dwinSendArray[0] == CMD_WRITE)
    {
        readDWIN();
    }
}

// Send int array to the display we dont need the 5A A5 or size - words only
void DWIN::sendIntArray(uint16_t instruction, uint16_t dwinIntArray[], byte arraySize)
{

    // turn our int array to array of bytes
    byte j = 0;
    byte dwinSendByteArray[arraySize];
    for (int i = 0; i < (arraySize >> 1); i++)
    {
        dwinSendByteArray[j] = (uint8_t)((dwinIntArray[i] >> 8) & 0xFF);
        j++;
        dwinSendByteArray[j] = (uint8_t)((dwinIntArray[i]) & 0xFF);
        j++;
    }

    byte sendBuffer[4 + sizeof(dwinSendByteArray)] = {CMD_HEAD1, CMD_HEAD2, (uint8_t)((arraySize + 1)), (uint8_t)((instruction) & 0xFF)};
    memcpy(sendBuffer + 4, dwinSendByteArray, sizeof(dwinSendByteArray));
    _dwinSerial->write(sendBuffer, sizeof(sendBuffer));

    // look for the ack. on write
    if ((uint8_t)((instruction) & 0xFF) == CMD_WRITE)
    { // or some others?
        readDWIN();
    }
}

// SET CallBack Event
void DWIN::hmiCallBack(hmiListener callBack)
{
    listenerCallback = callBack;
}

void DWIN::readDWIN()
{
    String buffer;
    TickType_t xLastWakeTime = xTaskGetTickCount();
    while (xTaskGetTickCount() - xLastWakeTime < pdMS_TO_TICKS(CMD_READ_TIMEOUT))
    {
        if (_dwinSerial->available() > 0)
        {
            // ...4F 4B -> 79, 75
            String c = String(_dwinSerial->read(), HEX);
            c.toUpperCase();
            buffer.concat(c);
            if (buffer.endsWith("4F4B"))
                break;
        }
    }

    handle();
}

String DWIN::checkHex(byte currentNo)
{
    if (currentNo < 0x10)
    {
        return "0" + String(currentNo, HEX);
    }
    return String(currentNo, HEX);
}

String DWIN::handle()
{
    int lastBytes;
    int previousByte;

    String address;
    String response;
    String message;

    bool isSubstr = false;
    bool messageEnd = false;
    bool isFirstByte = false;

    TickType_t xLastWakeTime = xTaskGetTickCount();
    while (xTaskGetTickCount() - xLastWakeTime < pdMS_TO_TICKS(READ_TIMEOUT))
    {
        while (_dwinSerial->available() > 0)
        {
            vTaskDelay(pdMS_TO_TICKS(10));

            int inhex = _dwinSerial->read();
            if (inhex == 90 || inhex == 165)
            { // 5A A5... -> 90, 165
                isFirstByte = true;
                message = "";
                response.concat(checkHex(inhex) + " ");
                continue;
            }
            for (int i = 1; i <= inhex; i++)
            {
                int inByte = _dwinSerial->read();
                if (i == 1)
                    response.concat(checkHex(inhex) + " ");
                if (i == (inhex - 1))
                    previousByte = inByte;
                response.concat(checkHex(inByte) + " ");
                if (i <= 3)
                {
                    if ((i == 2) || (i == 3))
                    {
                        address.concat(checkHex(inByte));
                    }
                    continue;
                }
                else
                {
                    if (messageEnd == false)
                    {
                        if (isSubstr && inByte != MAX_ASCII && inByte >= MIN_ASCII)
                        {
                            message += char(inByte);
                        }
                        else
                        {
                            if (inByte == MAX_ASCII)
                            {
                                messageEnd = true;
                            }
                            isSubstr = true;
                        }
                    }
                }
                lastBytes = inByte;
            }
        }
    }

    if (isFirstByte)
    {
        lastBytes = (previousByte << 8) + lastBytes;
        listenerCallback(address, lastBytes, message, response);
    }

    // if (isFirstByte)
    // {
    //     Serial.println("Address :0x" + address + " | Data :0x" + String(lastBytes, HEX) + " | Message : " + message + " | Response " + response);
    // }

    return response;
}

byte DWIN::readCMDLastByte(bool hiByte)
{
    byte lastByte = -1;
    byte previousByte = -1;

    TickType_t xLastWakeTime = xTaskGetTickCount();
    while (xTaskGetTickCount() - xLastWakeTime < pdMS_TO_TICKS(CMD_READ_TIMEOUT))
    {
        while (_dwinSerial->available() > 0)
        {
            previousByte = lastByte;
            lastByte = _dwinSerial->read();
        }
    }

    if (hiByte)
    {
        return previousByte;
    }
    else
    {
        return lastByte;
    }
}
