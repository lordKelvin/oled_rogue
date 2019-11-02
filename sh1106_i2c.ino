#include "sh1106_i2c.h"

void i2c_start()
{
  TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
  while(!(TWCR & (1 << TWINT)));
}

void i2c_stop()
{
  TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO);
}

void i2c_send(uint8_t data)
{
  TWDR = data;
  TWCR = (1 << TWINT) | (1 << TWEN);
  while(!(TWCR & (1 << TWINT)));
}

void display_init()
{
  const uint8_t init[] = {
    SH1106_I2C_ADDRESS,
    SH1106_COMMAND,
    SH1106_POWER + 0, // displayOff
    0x8D, 0x14, // Enable charge pump
    SH1106_RATIO_FREQUENCY, 0x80, // clockFrequency +15%
    SH1106_PUMP_VOLTAGE + 2, //...... pumpVoltage0123
    SH1106_LEFT_TO_RIGHT + 1, //...... segmentRemap
    SH1106_TOP_TO_BOTTOM + 8, //...... flipVertically
    // 0xDA, 0x12, // comConfiguration
    // 0xA8, 0x3F, // multiplex
    // 0xD3, 0x3F, // displayOffest
    SH1106_COLUMN_HIGH + 0, SH1106_COLUMN_LOW + 0, // columnAddr
    SH1106_START_LINE + 0, //...... startLine
    SH1106_PAGE + 0, //...... pageAddr
    SH1106_POWER + 1 //...... displayOn
  };
  i2c_start();
  for(uint8_t i = 0; i < sizeof(init); i++)
    i2c_send(init[i]);
  i2c_stop();
}

void clear_screen()
{
  for(uint8_t set_page = 0xB0; set_page < 0xB0 + 8; set_page++)
  {
    i2c_start();
    i2c_send(SH1106_I2C_ADDRESS);
    i2c_send(SH1106_COMMAND);
    i2c_send(SH1106_COLUMN_LOW + 0);
    i2c_send(SH1106_COLUMN_HIGH + 0);
    i2c_send(set_page);
    i2c_stop();

    i2c_start();
    i2c_send(SH1106_I2C_ADDRESS);
    i2c_send(SH1106_DATA);
    for(uint8_t x = 0; x < 128; x++)
      i2c_send(0x00);
    i2c_stop();
  }
}

void put_char(const char chr)
{
  i2c_start();
  i2c_send(SH1106_I2C_ADDRESS);
  i2c_send(SH1106_DATA); // data
  for(int i = 0; i < 5; i++)
    i2c_send(pgm_read_byte_near(font_5x8r + (chr - ' ') * 5 + i));
  i2c_stop();
}

void put_string(const char *str)
{
  for(const char *ptr = str; *ptr; ptr++)
    put_char(*ptr);
}

void set_scroll(signed char scroll)
{
  i2c_start();
  i2c_send(SH1106_I2C_ADDRESS);
  i2c_send(0x00); // command
  i2c_send(0x40 + scroll);
  i2c_stop();
}

// // uint8_t i2c_read(bool isLastByte)
// // {
// //   if(isLastByte) TWCR = (1 << TWINT) | (1 << TWEN);
// //   else TWCR = (1 << TWEA) | (1 << TWINT) | (1 << TWEN);
// //   while(!(TWCR & (1 << TWINT)));
// //   return TWDR;
// // }
