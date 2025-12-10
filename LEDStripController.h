// LEDStripController.h
/**
 * Brief   Controller for WS2812B RGB LED module (I2C / UART).
 * Author  StudentLED
 * Date    2025
 */

#ifndef _LEDSTRIPCONTROLLER_H
#define _LEDSTRIPCONTROLLER_H

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#include <Wire.h>

#define REG_LEDS_COUNTS                 0
#define REG_SET_LED_COLOR_DATA          1
#define REG_SET_LED_COLOR               2
#define REG_SET_ALL_LEDS_COLOR_DATA     3
#define REG_SET_ALL_LEDS_COLOR          4
#define REG_TRANS_DATA_TO_LED           5

#define REG_LEDS_COUNT_READ             0xfa
#define REG_READ_I2C_ADDRESS            0xfb
#define REG_GET_UART_BAUDRATE           0xfb
#define REG_SET_UART_BAUDRATE           0xfc
#define REG_SET_I2C_ADDRESS             0xfd
#define REG_GET_BRAND                   0xfe
#define REG_GET_FIRMWARE_VERSION        0xff

#define I2C_COMMUNICATION_MODE          0
#define UART_COMMUNICATION_MODE         1

#define STRING_BRAND_LENGTH             9
#define STRING_VERSION_LENGTH           16

#define SECRET_KEY_A                    0xaa
#define SECRET_KEY_B                    0xbb
#define UART_START_BYTE                 0xcc
#define UART_END_BYTE                   0xdd
#define UART_ACK_BYTE                   0x06

enum LED_TYPE
{
    TYPE_RGB = 0x06,
    TYPE_RBG = 0x09,
    TYPE_GRB = 0x12,
    TYPE_GBR = 0x21,
    TYPE_BRG = 0x18,
    TYPE_BGR = 0x24
};

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned long u32;

const u32 _BAUDRATE[] = {
    115200, 1200, 2400, 4800, 9600, 14400, 19200,
    38400, 57600, 115200, 128000, 230400, 500000
};

class LEDStripController
{
protected:
    u8 I2C_Address;
    u8 uartBaudrateIndex;
    u8 commMode;
    u16 ledCounts;
    u32 uartWaitAckTime;
    u8 rOffset;
    u8 gOffset;
    u8 bOffset;

    HardwareSerial *_serial;

    int writeRegOneByte(uint8_t val);
    int writeReg(uint8_t cmd, u8 *value, u8 size_a);
    int readReg(uint8_t cmd, char *recv, u16 count);
    bool uartWriteDataToControllerWithAck(u8 param[5], bool isShowLed = false);

public:
    LEDStripController(u8 _address = 0x20, u16 n = 8, LED_TYPE t = TYPE_GRB);
    LEDStripController(HardwareSerial *serial_param, u16 n = 8, LED_TYPE t = TYPE_GRB, u32 _baudrate = 115200);

    bool begin();
    bool setLedCount(u16 n);
    void setLedType(LED_TYPE t);

    bool setLedColorData(u8 index, u32 rgb);
    bool setLedColorData(u8 index, u8 r, u8 g, u8 b);

    bool setLedColor(u8 index, u32 rgb);
    bool setLedColor(u8 index, u8 r, u8 g, u8 b);

    bool setAllLedsColorData(u32 rgb);
    bool setAllLedsColorData(u8 r, u8 g, u8 b);

    bool setAllLedsColor(u32 rgb);
    bool setAllLedsColor(u8 r, u8 g, u8 b);

    bool show();

    u8 getLedsCountFromController();
    u8 getI2CAddress();
    u32 getUartBaudrate();
    bool setUartBaudrate(u32 _baudrate);
    bool setI2CNewAddress(u8 addr);
    String getBrand();
    String getFirmwareVersion();

    uint32_t Wheel(byte pos);
};

#endif
