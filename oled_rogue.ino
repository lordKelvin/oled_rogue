#include "U8glib.h"
#include "config.h"
// TODO: split into files

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
int dungeon_level = 0;
/* Player: finish */

char level[LEVEL_V][LEVEL_H];
char view[LEVEL_V][LEVEL_H + 1];

/* Monsters: start */

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
  bool alive, ccw;
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
      if(is_empty(monsters[i].x, monsters[i].y))
        break;
    }
    monsters[i].alive = true;
    monsters[i].ccw = random(2) == 1;
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

void xchg(int &a, int &b)
{
  int tmp = a;
  a = b;
  b = tmp;
}

void monster_step(int m) // TODO: use pointer to current monster
{
  Monsters &mon = monsters[m];
  if(mon.alive)
    monster_fight(m);
  else
    return;
  xchg(mon.dx, mon.dy);
  if(mon.ccw)
    mon.dx = -mon.dx;
  else
    mon.dy = -mon.dy;
  for(int j = 0; j < 4; j++) // 4 directions
  {
    if(mon.y + mon.dy < LEVEL_V - 1 && is_empty(mon.x + mon.dx, mon.y + mon.dy))
    {
      if(view[mon.y][mon.x] != ' ') // TODO: Fight here
        view[mon.y][mon.x] = mon.monster_clear;
      level[mon.y][mon.x] = mon.monster_clear;
      mon.y += mon.dy;
      mon.x += mon.dx;
      if(view[mon.y][mon.x] != ' ')
        view[mon.y][mon.x] = mon.monster_appearance;
      mon.monster_clear = level[mon.y][mon.x];
      level[mon.y][mon.x] = mon.monster_appearance;
      break;
    }
    else
    {
      xchg(mon.dx, mon.dy);
      if(mon.ccw)
        mon.dy = -mon.dy;
      else
        mon.dx = -mon.dx;
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
  u8g.setFont(u8g_font_5x8r);
  u8g.drawStr(0, n * 8 + 7, view[n]);
}

// https://bitbucket.org/eworoshow/maze/src/
void eller()
{
  int L[LEVEL_H / 2], R[LEVEL_H / 2]; // Set of cells: i links to R[i] (right). Similarly for the left

  for(int c = 0; c < LEVEL_H / 2; c++)
    L[c] = R[c] = c; // At the top each cell is connected only to itself

  for(int r = 0; r < LEVEL_V; r++)
  {
    for(int c = 0; c < LEVEL_H; c++)
    {
      if(r % 2 == 1 || c % 2 == 0)
        level[r][c] = '#';
      else
        level[r][c] = '.';
    }
    memset(view[r], ' ', LEVEL_H);
    view[r][LEVEL_H] = '\0';
  }

  for(int r = 0; r < LEVEL_V / 2 - 1; r++) // Generate each row of the maze excluding the last
  {
    for(int c = 0; c < LEVEL_H / 2; c++)
    {
      if(c != LEVEL_H / 2 - 1 && c + 1 != R[c] && random(2) == 0) // Should we connect this cell and its neighbour to the right?
      {
        R[L[c + 1]] = R[c]; // Link L[c + 1] to R[c]
        L[R[c]] = L[c + 1];
        R[c] = c + 1; // Link c to c + 1
        L[c + 1] = c;

        level[r * 2][c * 2 + 2] = '.';
      }

      if(c != R[c] && random(2) == 0) // Should we connect this cell and its neighbour below?
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

  for(int c = 0; c < LEVEL_H / 2; c++) // Handle the last row to guarantee the maze is connected
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
  strcpy(view[0] + 1, class_names[PC_Fighter]);
  strcpy(view[1] + 1, class_names[PC_Mage]);
  strcpy(view[2] + 1, class_names[PC_Thief]);
}

bool is_empty(int x, int y)
{
  if(x < 0 || y < 0 || x >= LEVEL_H || y >= LEVEL_V)
    return false;

  return (level[y][x] == '.' || level[y][x] == '>');
}

void show(void)
{
  for(int py = max(y - 2, 0); py < min(y + 2 + 1, LEVEL_V - 1); py++)
  {
    int tmpy = py;
    if(py < y) tmpy++;
    if(py > y) tmpy--;
    for(int px = max(x - 2, 0); px < min(x + 2 + 1, LEVEL_H); px++)
    {
      if(view[py][px] != ' ')
        continue;
      if(px == x - 2 || px == x + 2 || py == y - 2 || py == y + 2)
      {
        int tmpx = px;
        if(px < x) tmpx++;
        if(px > x) tmpx--;
        if(is_empty(tmpx, tmpy))
          view[py][px] = level[py][px];
      }
      else
      {
        view[py][px] = level[py][px];
      }
    }
  }
  sprintf(view[LEVEL_V - 1], "%d (%d, %d, %d) %d [%d]", hp, Strength, Intelligence, Agility, xp, dungeon_level);
}

void move(int dx, int dy)
{
  if(is_empty(x + dx, y + dy))
  {
    switch(level[y + dy][x + dx])
    {
    case '.': case '>':
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
      newlevel();
      break;
    }
    view[0][0] = (player_class == PC_Fighter ? '>' : ' ');
    view[1][0] = (player_class == PC_Mage ? '>' : ' ');
    view[2][0] = (player_class == PC_Thief ? '>' : ' ');
    break;
  case PS_Game:
    if(trigger_a) {
      if(player_clear == '>')
      {
        dungeon_level++;
        newlevel();
      }
    }
    if(trigger_b) {
      for(int y = 0; y < LEVEL_V - 1; y++)
        memcpy(view[y], level[y], LEVEL_H);
    }
    if(trigger_x) {
      dungeon_level++;
      newlevel();
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
    sprintf(view[1], " on %d-%s level of", dungeon_level, (dungeon_level == 1 ? "st" : (dungeon_level == 2 ? "nd" : (dungeon_level == 3 ? "rd" : "th"))));
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
