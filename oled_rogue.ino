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
int x = 2, y = 2;

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

void simulation_step()
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

void setup(void) {
  randomSeed(analogRead(0));
  pinMode(D2, INPUT_PULLUP);
  pinMode(D3, INPUT_PULLUP);
  pinMode(D4, INPUT_PULLUP);
  pinMode(D5, INPUT_PULLUP);
  pinMode(D6, INPUT_PULLUP);
  pinMode(D7, INPUT_PULLUP);
  pinMode(D8, INPUT_PULLUP);
  pinMode(D9, INPUT_PULLUP);
  for(int y = 0; y < LEVEL_V; y++)
  {
    for(int x = 0; x < LEVEL_H; x++)
    {
      level[y][x] = (random(10) < 4 ? '#' : '.');
    }
  }
  simulation_step();
  simulation_step();
  player_clear = level[y][x];
  level[y][x] = '@';
  show();
}

void show()
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
    case '.': case '<': case '>': case '/':
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
  buttons_states.d2 = (digitalRead(D2) == LOW);
  buttons_states.d3 = (digitalRead(D3) == LOW);
  buttons_states.d4 = (digitalRead(D4) == LOW);
  buttons_states.d5 = (digitalRead(D5) == LOW);
  buttons_states.d6 = (digitalRead(D6) == LOW);
  buttons_states.d7 = (digitalRead(D7) == LOW);
  buttons_states.d8 = (digitalRead(D8) == LOW);
  buttons_states.d9 = (digitalRead(D9) == LOW);

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
