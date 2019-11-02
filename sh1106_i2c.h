#pragma once
#include <stdint.h>
#include <avr/pgmspace.h>

// 0x3C << 1
#define SH1106_I2C_ADDRESS     0x78
#define SH1106_COMMAND         0x00
#define SH1106_DATA            0x40

// 4 bits
#define SH1106_COLUMN_LOW      0x00
// 4 bits
#define SH1106_COLUMN_HIGH     0x10
// 2 bits : 6.4, 7.4, 8, 9 V
#define SH1106_PUMP_VOLTAGE    0x30
// 6 bits
#define SH1106_START_LINE      0x40
// additional byte
#define SH1106_CONTRAST        0x81
// 1 bit
#define SH1106_LEFT_TO_RIGHT   0xA0
// 1 bit
#define SH1106_SHOW            0xA4
// 1 bit
#define SH1106_INVERT          0xA6
// additional byte, 6 bits
#define SH1106_MULTIPLEX       0xA8
// Set DC-DC OFF/ON: 0xAD, 0x8A + 1 bit
// 1 bit
#define SH1106_POWER           0xAE
// 3 bits
#define SH1106_PAGE            0xB0
// 3rd 1 bit
#define SH1106_TOP_TO_BOTTOM   0xC0
// Set Display Offset: 0xD3, 0x00 + 6 bits
#define SH1106_RATIO_FREQUENCY 0xD5

// extern const unsigned char font_5x8r[][5] PROGMEM;
extern const unsigned char font_5x8r[] PROGMEM;

