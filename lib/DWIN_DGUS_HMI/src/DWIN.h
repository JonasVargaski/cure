#ifndef DWIN_H
#define DWIN_H

#include "Arduino.h"

class DWIN
{

public:
    DWIN(HardwareSerial &port, uint8_t receivePin, uint8_t transmitPin, long baud);

    // Listen Touch Events & Messages from HMI
    String handle();
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
    // // Set Byte Data on VP Address makes more sense
    void setVPByte(long address, byte data); // alias of below
    void setVP(long address, byte data);
    // read byte from VP Address if bool = true read HiByte
    byte readVPByte(long address, bool = 0);
    // Set WordData on VP Address
    void setVPWord(long address, int data);
    // read WordData from VP Address you can read sequential multiple words (data returned in rx event)
    void readVPWord(long address, byte numWords);
    // read or write the NOR from/to VP must be on a even address 2 word are written or read
    void norReadWrite(bool write, long VPAddress, long NORAddress);
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

    // Callback Function
    typedef void (*hmiListener)(String address, int lastBytes, String message, String response);

    // CallBack Method
    void hmiCallBack(hmiListener callBackFunction);

private:
    Stream *_dwinSerial; // DWIN Serial interface

    hmiListener listenerCallback;

    byte readCMDLastByte(bool hiByte = 0);
    void readDWIN();
    String checkHex(byte currentNo);
};

#endif // DWIN_H