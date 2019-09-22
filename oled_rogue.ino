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
const char *stat_names[] = {"Strength", "Intelligence", "Agility"};
/* Menu: finish */

/* Player: start */
enum PlayerStates {
  PS_Menu = 0,
  PS_Game,
  PS_Inventory, // TODO: Win scene
  PS_Lose,
  PS_Win, // TODO: Levelup scene, on descention
} player_state = PS_Menu;

char player_clear;
int x, y, hp, maxhp, xp = 0;
int Strength, Intelligence, Agility;
int dungeon_level = 0; // TODO: increase (?)
/* Player: finish */

#define LEVEL_V 8
#define LEVEL_H 25

char level[LEVEL_V][LEVEL_H + 1];
char view[LEVEL_V][LEVEL_H + 1];

/* Monsters: start */
#define MAX_MONSTERS 10

enum MonsterClass {
  MC_Beast = 0,
  MC_Undead,
  MC_Flying
};

struct Monsters {
  int x, y, dx, dy;
  char monster_clear, monster_appearance;
  MonsterClass monster_class;
  int stat_value;
  bool alive;
  
  // TODO: add direction of turning, left or right
} monsters[MAX_MONSTERS];
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
    monsters[i].alive = true;
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
    switch(random(3))
    {
    case 0:
      monsters[i].monster_appearance = 'S';
      monsters[i].monster_class = MC_Beast;
      break;
    case 1:
      monsters[i].monster_appearance = 'Z';
      monsters[i].monster_class = MC_Undead;
      break;
    case 2:
      monsters[i].monster_appearance = 'F';
      monsters[i].monster_class = MC_Flying;
      break;
    }
    monsters[i].stat_value = random(dungeon_level) + 1;
    level[monsters[i].y][monsters[i].x] = monsters[i].monster_appearance;
  }
}

void monster_fight(int m)
{
  int h = sq(x - monsters[m].x) + sq(y - monsters[m].y);
  if(h > 2)
    return;
  bool Win = false;
  if(monsters[m].monster_class == MC_Undead && player_class == PC_Mage)
  {
    Win = true;
  }
  else
  {
    switch(monsters[m].monster_class)
    {
    case MC_Beast:
      if(Strength > monsters[m].stat_value)
        Win = true;
      break;
    case MC_Undead:
      if(Intelligence > monsters[m].stat_value)
        Win = true;
      break;
    case MC_Flying:
      if(Agility > monsters[m].stat_value)
        Win = true;
      break;
    }
  }
  xp += monsters[m].stat_value;
  if(!Win)
  {
    hp -= monsters[m].stat_value;
    if(hp <= 0)
      player_state = PS_Lose;
  }
  level[monsters[m].y][monsters[m].x] = monsters[m].monster_clear;
  monsters[m].alive = false;
  show();
}

void monster_step(int m) // TODO: use pointer to current monster
{
  if(monsters[m].alive)
    monster_fight(m);
  if(!monsters[m].alive)
    return;
  int tmp = monsters[m].dx; // Turn monster's direction clockwise
  monsters[m].dx = -monsters[m].dy;
  monsters[m].dy = tmp;
  for(int j = 0; j < 4; j++) // 4 directions
  {
    if(monsters[m].y + monsters[m].dy >= 0 && monsters[m].y + monsters[m].dy < LEVEL_V - 1 && level[monsters[m].y + monsters[m].dy][monsters[m].x + monsters[m].dx] == '.') // TODO: change to "is_empty"
    {
      if(view[monsters[m].y][monsters[m].x] != ' ')
        view[monsters[m].y][monsters[m].x] = monsters[m].monster_clear;
      level[monsters[m].y][monsters[m].x] = monsters[m].monster_clear;
      monsters[m].y += monsters[m].dy;
      monsters[m].x += monsters[m].dx;
      if(view[monsters[m].y][monsters[m].x] != ' ')
        view[monsters[m].y][monsters[m].x] = monsters[m].monster_appearance;
      monsters[m].monster_clear = level[monsters[m].y][monsters[m].x];
      level[monsters[m].y][monsters[m].x] = monsters[m].monster_appearance;
      break;
    }
    else
    {
      tmp = -monsters[m].dx; // Turn monster's direction counterclockwise
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
  if(monster_count < MAX_MONSTERS - 1)
    monster_count++;
  x = random(LEVEL_H / 2) * 2 + 1;
  y = random(LEVEL_V / 2) * 2;
  level[random(LEVEL_V / 2) * 2][random(LEVEL_H / 2) * 2 + 1] = '>'; // TODO: find a dead-end
  player_clear = level[y][x];
  level[y][x] = '@';
  generate_monsters();
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
  for(int py = max(y - 1, 0); py < min(y + 2, LEVEL_V - 1); py++)
  {
    for(int px = max(x - 1, 0); px < min(x + 2, LEVEL_H); px++)
    {
      view[py][px] = level[py][px];
    }
  }
  sprintf(view[LEVEL_V - 1], "%d (%d, %d, %d) %d [%d]", hp, Strength, Intelligence, Agility, xp, dungeon_level);
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
      dungeon_level++;
      newlevel();
      break;
    case '.':
      level[y][x] = player_clear;
      x += dx; // TODO: count steps, heal up
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
      switch(player_class)
      {
      case PC_Fighter:
        Strength = 2, Intelligence = 2, Agility = 2, hp = 50;
        break;
      case PC_Mage:
        Strength = 1, Intelligence = 4, Agility = 1, hp = 10;
        break;
      case PC_Thief:
        Strength = 2, Intelligence = 2, Agility = 4, hp = 20;
        break;
      }
      break;
    }
    view[0][0] = (player_class == PC_Fighter ? '>' : ' ');
    view[1][0] = (player_class == PC_Mage ? '>' : ' ');
    view[2][0] = (player_class == PC_Thief ? '>' : ' ');
    break;
  case PS_Game:
    if(trigger_a) {
      for(int y = 0; y < LEVEL_V - 1; y++)
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
  case PS_Lose:
    sprintf(view[0], "You lost your life");
    sprintf(view[1], " on %d-th level of", dungeon_level);
    sprintf(view[2], " gloomy dungeon");
    sprintf(view[3], " scoring %d", xp);
    sprintf(view[4], " expirience points.");
    view[5][0] = view[6][0] = view[7][0] = '\0';
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
