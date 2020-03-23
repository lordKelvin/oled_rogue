#pragma once
#include <stdint.h>
#include <avr/pgmspace.h>

// 0x50 << 1
#define EEPROM_I2C_ADDRESS     0xA0

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
#define SH1106_CONTRAST        0x81u
// additional byte
#define SH1106_CHARGE_PUMP     0x8Du
// 1 bit
#define SH1106_LEFT_TO_RIGHT   0xA0u
// 1 bit
#define SH1106_SHOW            0xA4u
// 1 bit
#define SH1106_INVERT          0xA6u
// additional byte, 6 bits
#define SH1106_MULTIPLEX       0xA8u
// Set DC-DC OFF/ON: 0xAD, 0x8A + 1 bit
// 1 bit
#define SH1106_POWER           0xAEu
// 3 bits
#define SH1106_PAGE            0xB0u
// 3rd 1 bit
#define SH1106_TOP_TO_BOTTOM   0xC0u
// Set Display Offset: 0xD3, 0x00 + 6 bits
#define SH1106_RATIO_FREQUENCY 0xD5u

extern const unsigned char font_5x8r[] PROGMEM;
