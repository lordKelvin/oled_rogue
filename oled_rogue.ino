#include "U8glib.h"

#define D2 2
#define D3 3
#define D4 4
#define D5 5
#define D6 6
#define D7 7
#define D8 8
#define D9 9

struct MyButtons {
  unsigned int d2 : 1;
  unsigned int d3 : 1;
  unsigned int d4 : 1;
  unsigned int d5 : 1;
  unsigned int d6 : 1;
  unsigned int d7 : 1;
  unsigned int d8 : 1;
  unsigned int d9 : 1;
};
MyButtons buttons_previous;
MyButtons buttons_states = {false, false, false, false, false, false, false, false};

char player_clear = '.';
int x, y;

char level[8][26];
char view[8][26];

#define LEVEL_V 8
#define LEVEL_H 25


int count_neighbours(int x, int y)
{
  int count = 0;
  if(y == 0) count += 3;
  if(y == LEVEL_V - 1) count += 3;
  if(x == 0) count += 3;
  if(x == LEVEL_H - 1) count += 3;
  for(int py = max(y - 1, 0); py < min(y + 2, LEVEL_V); py++)
  {
    for(int px = max(x - 1, 0); px < min(x + 2, LEVEL_H); px++)
    {
      if(level[py][px] == '#')
        count++;
    }
  }
  return count;
}

void simulation_step(void)
{
  for(int y = 0; y < LEVEL_V; y++)
  {
    for(int x = 0; x < LEVEL_H; x++)
    {
      int nbs = count_neighbours(x, y);
      if(level[y][x] == '#')
      {
        view[y][x] = ((nbs < 3) ? '.' : '#');
      }
      else
      {
        view[y][x] = ((nbs > 4) ? '#' : '.');
      }
    }
  }

  for(int y = 0; y < LEVEL_V; y++)
  {
    for(int x = 0; x < LEVEL_H; x++)
    {
      level[y][x] = view[y][x];
      view[y][x] = ' ';
    }
    level[y][LEVEL_H] = view[y][LEVEL_H] = '\0';
  }
}

U8GLIB_SH1106_128X64 u8g(U8G_I2C_OPT_DEV_0|U8G_I2C_OPT_FAST);                     // I2C / TWI 1.3

void draw(int n) {
  u8g.setFont(u8g_font_5x8);
  u8g.drawStr(0, n * 8 + 8, view[n]);
}

void newlevel(void)
{
  for(int y = 0; y < LEVEL_V; y++)
  {
    for(int x = 0; x < LEVEL_H; x++)
    {
      level[y][x] = (random(10) < 4 ? '#' : '.');
    }
  }
  simulation_step();
  simulation_step();
  x = random(LEVEL_H - 4) + 2;
  y = random(LEVEL_V - 4) + 2;
  level[random(LEVEL_V - 4) + 2][random(LEVEL_H - 4) + 2] = '>';
  player_clear = level[y][x];
  level[y][x] = '@';
  show();
}

void setup(void)
{
  randomSeed(analogRead(0));
  DDRD &= ~bit(D2); PORTD |= bit(D2); // set digital pin 2 as INPUT_PULLUP
  DDRD &= ~bit(D3); PORTD |= bit(D3); // set digital pin 3 as INPUT_PULLUP
  DDRD &= ~bit(D4); PORTD |= bit(D4); // set digital pin 4 as INPUT_PULLUP
  DDRD &= ~bit(D5); PORTD |= bit(D5); // set digital pin 5 as INPUT_PULLUP
  DDRD &= ~bit(D6); PORTD |= bit(D6); // set digital pin 6 as INPUT_PULLUP
  DDRD &= ~bit(D7); PORTD |= bit(D7); // set digital pin 7 as INPUT_PULLUP
  DDRB &= ~bit(0); PORTB |= bit(0); // set digital pin 8 as INPUT_PULLUP
  DDRB &= ~bit(1); PORTB |= bit(1); // set digital pin 9 as INPUT_PULLUP
  newlevel();
}

void show(void)
{
  for(int py = max(y - 1, 0); py < min(y + 2, LEVEL_V); py++)
  {
    for(int px = max(x - 1, 0); px < min(x + 2, LEVEL_H); px++)
    {
      view[py][px] = level[py][px];
    }
  }
}

void move(int dx, int dy)
{
  if(x + dx >= 0 && x + dx < LEVEL_H && y + dy >= 0 && y + dy < LEVEL_V)
  {
    switch(level[y + dy][x + dx])
    {
    case '#':
      break;
    case '>':
      newlevel();
      break;
    case '.':
      level[y][x] = player_clear;
      x += dx;
      y += dy;
      player_clear = level[y][x];
      level[y][x] = '@';
      show();
      break;
    }
  }
}

void loop(void) {
  // https://robotic-controls.com/learn/optimized-multiple-pin-reads
  // https://www.arduino.cc/en/Reference/PortManipulation
  // https://gist.github.com/nadavmatalon/08e4ab2ca1d0958c551a89ce3cb36d90
  buttons_states.d2 = ((PIND & 0b00000100) == 0);
  buttons_states.d3 = ((PIND & 0b00001000) == 0);
  buttons_states.d4 = ((PIND & 0b00010000) == 0);
  buttons_states.d5 = ((PIND & 0b00100000) == 0);
  buttons_states.d6 = ((PIND & 0b01000000) == 0);
  buttons_states.d7 = ((PIND & 0b10000000) == 0);
  buttons_states.d8 = ((PINB & 0b00000001) == 0);
  buttons_states.d9 = ((PINB & 0b00000010) == 0);

  if(buttons_states.d2 && !buttons_previous.d2) {
    // a
  }
  if(buttons_states.d3 && !buttons_previous.d3) {
     // b
  }
  if(buttons_states.d4 && !buttons_previous.d4) {
     // x
  }
  if(buttons_states.d5 && !buttons_previous.d5) {
     // y
  }
  if(buttons_states.d6 && !buttons_previous.d6) {
    move(-1, 0);
  }
  if(buttons_states.d7 && !buttons_previous.d7) {
    move(0, -1);
  }
  if(buttons_states.d8 && !buttons_previous.d8) {
    move(+1, 0);
  }
  if(buttons_states.d9 && !buttons_previous.d9) {
    move(0, +1);
  }

  buttons_previous = buttons_states;

  int n = 0;
  u8g.firstPage();
  do {
    draw(n);
    n++;
  } while(u8g.nextPage()); 
}
