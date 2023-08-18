#include "stdarg.h"
#include "sh1106_i2c.h"

#define F_SCL 400000L // SCL frequency

// https://chipenable.ru/index.php/programming-avr/195-uchebnyy-kurs-avr-ispolzovaniya-twi-modulya-registry-ch2.html
// http://arduino.ru/forum/programmirovanie/itg3205-problema-polucheniya-dannykh
void i2c_init()
{
  // https://users.soe.ucsc.edu/~karplus/Arduino/libraries/i2c/i2c.cpp
  uint8_t prescale = ((1 + (TWSR & 3)) << 1);
  TWBR = ((F_CPU / F_SCL) - 16) >> prescale;
}

void i2c_start()
{
//  TWCR = 0; // reset TWI control register
  TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN); // transmit START condition
  while(!(TWCR & (1 << TWINT))); // wait for end of transmission

//  check if the start condition was successfully transmitted
//  if((TWSR & 0xF8) != TW_START){ return false; }
  // TODO: add i2c_send here, it's always used
}

void i2c_stop()
{
  TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO); // send stop condition
//  while(TWCR & (1 << TWSTO)); // wait until stop condition is executed and bus released
}

void i2c_send(uint8_t data)
{
  TWDR = data;
  TWCR = (1 << TWINT) | (1 << TWEN); // start transmission
  while(!(TWCR & (1 << TWINT))); // wait for end of transmission
//  if( (TWSR & 0xF8) != TW_MT_DATA_ACK ){ return false; }
}

void display_init()
{
  const uint8_t init[] = {
    SH1106_I2C_ADDRESS,
    SH1106_COMMAND,
    SH1106_POWER + 0, // displayOff
    SH1106_CHARGE_PUMP, 0x14, // Enable charge pump
    SH1106_RATIO_FREQUENCY, 0x80, // clockFrequency +15%
    SH1106_PUMP_VOLTAGE + 2, //...... pumpVoltage 8V
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
  for(uint8_t set_page = SH1106_PAGE; set_page < SH1106_PAGE + 8; set_page++)
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

void set_scroll(uint8_t scroll)
{
  i2c_start();
  i2c_send(SH1106_I2C_ADDRESS);
  i2c_send(SH1106_COMMAND); // command
  i2c_send(SH1106_START_LINE + scroll);
  i2c_stop();
}

void set_char(const uint8_t x, const uint8_t y, const char chr)
{
  if(chr < ' ' || chr >= 128)
    return;
  // TODO: test x and y?
  uint8_t xlo = (x * 5) & 0xF;
  uint8_t xhi = (x * 5) >> 4;
  i2c_start();
  i2c_send(SH1106_I2C_ADDRESS);
  i2c_send(SH1106_COMMAND);
  i2c_send(SH1106_COLUMN_LOW + xlo);
  i2c_send(SH1106_COLUMN_HIGH + xhi);
  i2c_send(SH1106_PAGE + y);
  i2c_stop();
  i2c_start();
  i2c_send(SH1106_I2C_ADDRESS);
  i2c_send(SH1106_DATA);
  for(int i = 0; i < 5; i++)
    i2c_send(pgm_read_byte_near(font_5x8r + (chr - ' ') * 5 + i));
  i2c_stop();
}

// TODO: overflow
void set_string(const uint8_t x, const uint8_t y, const char *str)
{
  if(str[0] == '\0')
    return;
  set_char(x, y, str[0]);

  i2c_start();
  i2c_send(SH1106_I2C_ADDRESS);
  i2c_send(SH1106_DATA);
  for(const char *ptr = str + 1; *ptr; ptr++)
    for(int i = 0; i < 5; i++)
      i2c_send(pgm_read_byte_near(font_5x8r + (*ptr - ' ') * 5 + i));
  i2c_stop();
}

void set_printf(uint8_t x, uint8_t y, const char* format, ...)
{
  char buf[LEVEL_H + 1];
  va_list argptr;
  va_start(argptr, format);
  vsnprintf(buf, LEVEL_H + 1, format, argptr);
  va_end(argptr);
  set_string(x, y, buf);
}

uint8_t i2c_read()
{
  TWCR = (1 << TWEA) | (1 << TWINT) | (1 << TWEN);
  while(!(TWCR & (1 << TWINT)));
  return TWDR;
}

uint8_t i2c_read_stop()
{
  TWCR = (1 << TWINT) | (1 << TWEN);
  while(!(TWCR & (1 << TWINT)));
  return TWDR;
}

void display_toggle(int on)
{
  i2c_start();
  i2c_send(SH1106_I2C_ADDRESS);
  i2c_send(SH1106_COMMAND);
  i2c_send(SH1106_POWER + on);
  i2c_stop();
}

void EEPROM_WriteByte(uint8_t eeprom_device_address, uint16_t eeprom_address, uint8_t value)
{
  i2c_start();
  i2c_send(eeprom_device_address << 1);
//  i2c_start_addr(eeprom_device_address << 1);
  i2c_send((uint8_t)(eeprom_address >> 8));
  i2c_send((uint8_t)(eeprom_address & 0xFF));
  i2c_send(value);
  i2c_stop();
  delay(10);
}

// E2END: The last EEPROM address.
void eeprom_write(uint8_t *data, uint16_t len, uint16_t addr)
{
//  if(len == 0)
//    return;
//  
//  i2c_start();
//  // i2c_addr(EEPROM_I2C_ADDRESS << 1);
//  i2c_send(EEPROM_I2C_ADDRESS << 1);
//  i2c_send((uint8_t)(addr >> 8));
//  i2c_send((uint8_t)(addr & 0xFF));
//  for(uint16_t i = 0; i < len; i++)
//    i2c_send(data[i]);
//  i2c_stop();
//  delay(10);
  for(uint16_t i = 0; i < len; i++)
    EEPROM_WriteByte(EEPROM_I2C_ADDRESS, addr + i, data[i]);
}

void eeprom_read(uint8_t *data, uint16_t len, uint16_t addr)
{
  if(len == 0)
    return;

  i2c_start();
  // i2c_addr(EEPROM_I2C_ADDRESS << 1);
  i2c_send(EEPROM_I2C_ADDRESS << 1);
  i2c_send((uint8_t)(addr >> 8));
  i2c_send((uint8_t)(addr & 0xFF));
  for(uint16_t i = 0; i < len - 1; i++)
    data[i] = i2c_read();
  data[len - 1] = i2c_read_stop();
  i2c_stop();
}
