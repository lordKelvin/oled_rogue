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

/* Menu: start */
enum PlayerClass {
  PC_Fighter = 0,
  PC_Mage = 1,
  PC_Thief = 2
};
int player_class = PC_Fighter;
const char *class_names[] = {"Fighter", "Mage", "Thief"};
/* Menu: finish */

/* Player: start */
enum PlayerStates {
  PS_Menu = 0,
  PS_Game,
  PS_Inventory
} player_state = PS_Menu;

char player_clear;
int x, y;
/* Player: finish */

#define LEVEL_V 8
#define LEVEL_H 25

char level[LEVEL_V][LEVEL_H + 1];
char view[LEVEL_V][LEVEL_H + 1];

/* Monsters: start */
#define MAX_MONSTERS 10
struct Monsters {
  int x, y, dx, dy;
  char monster_clear;
  // TODO: add direction of turning, left or right
};

Monsters monsters[MAX_MONSTERS];
int monster_count = 0;

void generate_monsters()
{
  for(int i = 0; i < monster_count; i++)
  {
    while(true) // TODO: if too many attempts - stop
    {
      monsters[i].x = random(LEVEL_H / 2) * 2 + 1;
      monsters[i].y = random(LEVEL_V / 2) * 2;
      if(level[monsters[i].y][monsters[i].x] == '.') // TODO: change to "is_empty"
        break;
    }
    monsters[i].dx = monsters[i].dy = 0;
    switch(random(4))
    {
    case 0:
      monsters[i].dx = +1;
      break;
    case 1:
      monsters[i].dy = +1;
      break;
    case 2:
      monsters[i].dx = -1;
      break;
    case 3:
      monsters[i].dy = -1;
      break;
    }
    monsters[i].monster_clear = level[monsters[i].y][monsters[i].x];
    level[monsters[i].y][monsters[i].x] = 'S'; // TODO: add more monster types
  }
}

void monster_step(int m)
{
  int tmp = monsters[m].dx;
  monsters[m].dx = -monsters[m].dy;
  monsters[m].dy = tmp;
  for(int j = 0; j < 4; j++)
  {
    if(monsters[m].y + monsters[m].dy >= 0 && level[monsters[m].y + monsters[m].dy][monsters[m].x + monsters[m].dx] == '.') // TODO: change to "is_empty"
    {
      if(view[monsters[m].y][monsters[m].x] != ' ')
        view[monsters[m].y][monsters[m].x] = monsters[m].monster_clear;
      level[monsters[m].y][monsters[m].x] = monsters[m].monster_clear;
      monsters[m].y += monsters[m].dy;
      monsters[m].x += monsters[m].dx;
      if(view[monsters[m].y][monsters[m].x] != ' ')
        view[monsters[m].y][monsters[m].x] = 'S';
      monsters[m].monster_clear = level[monsters[m].y][monsters[m].x];
      level[monsters[m].y][monsters[m].x] = 'S';
      break;
    }
    else
    {
      tmp = -monsters[m].dx;
      monsters[m].dx = monsters[m].dy;
      monsters[m].dy = tmp;
    }
  }
}

void monsters_step()
{
  for(int i = 0; i < monster_count; i++)
  {
    monster_step(i);
  }
}
/* Monsters:finish */

U8GLIB_SH1106_128X64 u8g(U8G_I2C_OPT_DEV_0 | U8G_I2C_OPT_FAST); // I2C / TWI 1.3

void draw(int n)
{
  u8g.setFont(u8g_font_5x8);
  u8g.drawStr(0, n * 8 + 8, view[n]);
}

void eller()
{
    // For a set of cells i_1, i_2, ..., i_k from left to right that are connected in the previous row R[i_1] = i_2, R[i_2] = i_3, ... and R[i_k] = i_1. Similarly for the left
    int L[LEVEL_H / 2], R[LEVEL_H / 2];

    // At the top each cell is connected only to itself
    for(int c = 0; c < LEVEL_H / 2; c++)
        L[c] = R[c] = c;

    for(int r = 0; r < LEVEL_V; r++)
    {
        for(int c = 0; c < LEVEL_H; c++)
        {
            if(r % 2 == 1 || c % 2 == 0)
                level[r][c] = '#';
            else
                level[r][c] = '.';
        }
        level[r][LEVEL_H] = '\0';
        memset(view[r], ' ', LEVEL_H);
        view[r][LEVEL_H] = '\0';
    }

    // Generate each row of the maze excluding the last
    for(int r = 0; r < LEVEL_V / 2 - 1; r++)
    {
        for(int c = 0; c < LEVEL_H / 2; c++)
        {
            // Should we connect this cell and its neighbour to the right?
            if(c != LEVEL_H / 2 - 1 && c + 1 != R[c] && random(2) == 0)
            {
                R[L[c + 1]] = R[c]; // Link L[c + 1] to R[c]
                L[R[c]] = L[c + 1];
                R[c] = c + 1; // Link c to c + 1
                L[c + 1] = c;

                level[r * 2][c * 2 + 2] = '.';
            }

            // Should we connect this cell and its neighbour below?
            if(c != R[c] && random(2) == 0)
            {
                R[L[c]] = R[c]; // Link L[c] to R[c]
                L[R[c]] = L[c];
                R[c] = c; // Link c to c
                L[c] = c;
            } else {
                level[r * 2 + 1][c * 2 + 1] = '.';
            }
        }
    }

    // Handle the last row to guarantee the maze is connected
    for(int c = 0; c < LEVEL_H / 2; c++)
    {
        if(c != LEVEL_H / 2 - 1 && c + 1 != R[c] && (c == R[c] || random(2) == 0))
        {
            R[L[c + 1]] = R[c]; // Link L[c + 1] to R[c]
            L[R[c]] = L[c + 1];
            R[c] = c + 1; // Link c to c + 1
            L[c + 1] = c;

            level[(LEVEL_V / 2 - 1) * 2][c * 2 + 2] = '.';
        }

        R[L[c]] = R[c]; // Link L[c] to R[c]
        L[R[c]] = L[c];
        R[c] = L[c] = c; // Link c to c
    }
}

void newlevel(void)
{
  eller();
  monster_count++;
  generate_monsters();
  x = random(LEVEL_H / 2) * 2 + 1;
  y = random(LEVEL_V / 2) * 2;
  level[random(LEVEL_V / 2) * 2][random(LEVEL_H / 2) * 2 + 1] = '>'; // TODO: find a dead-end
  player_clear = level[y][x];
  level[y][x] = '@';
  show();
}

void setup(void)
{
  randomSeed(analogRead(0));
  DDRD &= ~0b11111100; PORTD |= 0b11111100; // set digital pin 2-7 as INPUT_PULLUP
  DDRB &= ~0b00000011; PORTB |= 0b00000011; // set digital pin 8-9 as INPUT_PULLUP
  // newlevel();
  strcpy(view[0] + 1, class_names[PC_Fighter]);
  strcpy(view[1] + 1, class_names[PC_Mage]);
  strcpy(view[2] + 1, class_names[PC_Thief]);
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
  monsters_step();
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

  bool trigger_a = (buttons_states.d2 && !buttons_previous.d2);
  bool trigger_b = (buttons_states.d3 && !buttons_previous.d3);
  bool trigger_x = (buttons_states.d4 && !buttons_previous.d4);
  bool trigger_y = (buttons_states.d5 && !buttons_previous.d5);
  bool trigger_left = (buttons_states.d6 && !buttons_previous.d6);
  bool trigger_up = (buttons_states.d7 && !buttons_previous.d7);
  bool trigger_right = (buttons_states.d8 && !buttons_previous.d8);
  bool trigger_down = (buttons_states.d9 && !buttons_previous.d9);

  switch(player_state)
  {
  case PS_Menu:
    if(trigger_up) {
      if(player_class != PC_Fighter)
        player_class--;
    }
    if(trigger_down) {
      if(player_class != PC_Thief)
        player_class++;
    }
    if(trigger_a) {
      player_state = PS_Game;
      newlevel();
      break;
    }
    view[0][0] = (player_class == PC_Fighter ? '>' : ' ');
    view[1][0] = (player_class == PC_Mage ? '>' : ' ');
    view[2][0] = (player_class == PC_Thief ? '>' : ' ');
    break;
  case PS_Game:
    if(trigger_a) {
      for(int y = 0; y < LEVEL_V; y++)
        memcpy(view[y], level[y], LEVEL_H);
    }
    if(trigger_left) {
      move(-1, 0);
    }
    if(trigger_up) {
      move(0, -1);
    }
    if(trigger_right) {
      move(+1, 0);
    }
    if(trigger_down) {
      move(0, +1);
    }
    break;
  case PS_Inventory:
    break;
  }

  buttons_previous = buttons_states;

  int n = 0;
  u8g.firstPage();
  do {
    draw(n);
    n++;
  } while(u8g.nextPage()); 
}
