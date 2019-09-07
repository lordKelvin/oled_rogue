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

#define LEVEL_V 8
#define LEVEL_H 25

char level[LEVEL_V][LEVEL_H + 1];
char view[LEVEL_V][LEVEL_H + 1];

int count_neighbours(int x, int y)
{
  int count = 0;
  int minx = x, maxx = x, miny = y, maxy = y;

  if(x == 0)
    count += 3;
  else
    minx = x - 1;
  if(x == LEVEL_H - 1)
    count += 3;
  else
    maxx = x + 1;

  if(y == 0)
    count += 3;
  else
    miny = y - 1;
  if(y == LEVEL_V - 1)
    count += 3;
  else
    maxy = y + 1;

  if(count == 6)
    count--;

  for(int py = miny; py <= maxy; py++)
    for(int px = minx; px < maxx; px++)
      if(level[py][px] == '#')
        count++;

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
    memcpy(level[y], view[y], LEVEL_H + 1);
    memset(view[y], ' ', LEVEL_H);
  }
}

U8GLIB_SH1106_128X64 u8g(U8G_I2C_OPT_DEV_0 | U8G_I2C_OPT_FAST); // I2C / TWI 1.3

void draw(int n)
{
  u8g.setFont(u8g_font_5x8);
  u8g.drawStr(0, n * 8 + 8, view[n]);
}

void newlevel(void)
{
  int r = random(LEVEL_V - 4) + 2;
  for(int y = 0; y < LEVEL_V; y++)
  {
    for(int x = 0; x < LEVEL_H; x++)
    {
      // level[y][x] = ((y != r && random(11) < 4) ? '#' : '.');
      level[y][x] = ((y != r && random(10) < 4) ? '#' : '.');
    }
    view[y][LEVEL_H] = '\0';
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
  DDRD &= ~0b11111100; PORTD |= 0b11111100; // set digital pin 2-7 as INPUT_PULLUP
  DDRB &= ~0b00000011; PORTB |= 0b00000011; // set digital pin 8-9 as INPUT_PULLUP
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
//    case '#':
//      break;
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
    for(int y = 0; y < LEVEL_V; y++)
      memcpy(view[y], level[y], LEVEL_H);
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
