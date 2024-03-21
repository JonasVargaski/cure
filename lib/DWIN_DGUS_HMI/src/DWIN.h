#ifndef DWIN_H
#define DWIN_H

#include "Arduino.h"
#include "DWIN_FRAME.h"

class DWIN
{

public:
    DWIN(HardwareSerial &port, uint8_t receivePin, uint8_t transmitPin, long baud, int bufferSize);

    // Listen Touch Events & Messages from HMI
    DwinFrame *handle();
    // Get Version
    double getHWVersion();
    // get GUI software version
    double getGUISoftVersion();
    // restart HMI
    void restartHMI();
    // set Particular Page
    void setPage(byte pageID);
    // get Current Page ID
    byte getPage();
    // set LCD Brightness
    void setBrightness(byte pConstrast);
    // set LCD Brightness
    byte getBrightness();
    // set Data on VP Address
    void setText(long address, String textData);
    // Set WordData on VP Address
    void setVPWord(long address, int data);
    // read WordData from VP Address you can read sequential multiple words (data returned in rx event)
    void readVPWord(uint16_t address, byte numWords);
    // Play a sound
    void playSound(byte soundID);
    // beep Buzzer for 1 sec
    void beepHMI();
    // set the hardware RTC The first two digits of the year are automatically added
    void setRTC(byte year, byte month, byte day, byte hour, byte minute, byte second);
    // update the software RTC The first two digits of the year are automatically added
    void setRTCSOFT(byte year, byte month, byte day, byte weekday, byte hour, byte minute, byte second);
    // set text color (16-bit RGB) on controls which allow it ie. text control.
    //  changes the control sp address space (sp=description pointer) content see the DWIN docs.
    void setTextColor(long spAddress, long spOffset, long color);
    // set float value to 32bit DATA Variable Control
    void setFloatValue(long vpAddress, float fValue);

    // Send array to the display we dont need the 5A A5 or
    // the size byte hopefully we can work this out.
    // byte hmiArray[] = {0x83,0x10,0x00,0x1};        // Read 0x1000 one word returns in the rx event
    // byte hmiArray[] = {0x82,0x88,0x00,0x55,0xAA};  // Write 0x8800
    // hmi.sendArray(hmiArray,sizeof(hmiArray));
    void sendArray(byte dwinSendArray[], byte arraySize);

    // Send int array to the display we dont need the 5A A5 or size - words only
    // eg. Using Basic Graphic Control vp 0x5000 draw rectangle
    //  uint16_t intArrayRect[] = {0x5000,0x0003,0x0001,200,100,650,400,0xFFF0,0xFF00};
    //  Fill it with Yellow
    //  uint16_t intArrayFill[] = {0x5000,0x0004,0x0001,200,100,650,400,0xFFF0,0xFF00};
    //  display it
    //  hmi.sendIntArray(0x82,intArrayRect,sizeof(intArrayRect));
    //  hmi.sendIntArray(0x82,intArrayFill,sizeof(intArrayFill));
    void sendIntArray(uint16_t instruction, uint16_t dwinIntArray[], byte arraySize);

    typedef void (*hmiListener)(DwinFrame *frame);
    void hmiCallBack(hmiListener callBackFunction);

private:
    Stream *_dwinSerial;
    DwinFrame frame;
    hmiListener listenerCallback;

    DwinFrame *readDWIN();
};

#endif // DWIN_H