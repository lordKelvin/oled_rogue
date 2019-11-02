//#include <U8glibmin.h>
#include "sh1106_i2c.h"
#include "config.h"
#include "monsters.h"

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
unsigned long timeout;
bool power_save = false;
int shake = 0;

/* Menu: start */
enum PlayerClass {
  PC_Fighter = 0,
  PC_Mage = 1,
  PC_Thief = 2
};
int current_menu = 0;
const char *class_names[] = {"Fighter", "Mage", "Thief", NULL};
const char *stat_names[] = {"Strength", "Intelligence", "Agility", NULL};
/* Menu: finish */

/* Player: start */
enum PlayerStates {
  PS_Menu = 0,
  PS_Game,
  PS_Inventory, // TODO: Win scene
  PS_Lose,
  PS_Win,
  PS_Levelup,
} player_state = PS_Menu;

int player_class;
char player_clear;
int x, y, hp, maxhp, xp = 0, maxxp = 1, xptier = 1;
int Strength, Intelligence, Agility;
int dungeon_level = 0;
/* Player: finish */

char level[LEVEL_V][LEVEL_H];
char view[LEVEL_V][LEVEL_H + 1];

// U8GLIB_SH1106_128X64 u8g(U8G_I2C_OPT_FAST); // I2C / TWI 1.3

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
      view[r][c] = ' ';
    }
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
  level[random(LEVEL_V / 2) * 2][random(LEVEL_H / 2) * 2 + 1] = '>';
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
  // u8g.setFont(u8g_font_5x8r);
  display_init();
  clear_screen();
  timeout = millis();
//  Serial.begin(9600);
//  #ifdef __AVR__
//    Serial.println(__AVR__);
//  #endif
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
  sprintf(view[LEVEL_V - 1], "%d (%d, %d, %d) %d/%d [%d]", hp, Strength, Intelligence, Agility, xp, maxxp, dungeon_level);
}

void move(int dx, int dy)
{
  if(is_empty(x + dx, y + dy))
  {
    level[y][x] = player_clear;
    x += dx;
    y += dy;
    player_clear = level[y][x];
    level[y][x] = '@';
    show();
  }
  monsters_step();
}

int question(const char **options, bool sub_item, bool add_item, bool confirm)
{
  static int current_item = 0, count = -1;
  if(count == -1)
  {
    current_item = 0;
    for(count = 0; options[count]; count++)
      strcpy(view[count] + 1, options[count]);
  }
  if(sub_item && current_item > 0)
    current_item--;
  if(add_item && current_item < count - 1)
    current_item++;
  for(int i = 0; i < count; i++)
    view[i][0] = (current_item == i ? '>' : ' ');
  if(confirm)
  {
    count = -1;
    return current_item;
  }
  else
    return -1;
}

void loop(void)
{
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

  buttons_previous = buttons_states;

  {
    unsigned long current_time = millis();
    if(trigger_left || trigger_up || trigger_right || trigger_down || trigger_left || trigger_up || trigger_right || trigger_down)
    {
      if(power_save)
      {
        power_save = false;
//        u8g.sleepOff();
      }
      timeout = current_time;
    }
    else if(current_time - timeout > SCREENSAVER_TIMEOUT_MS)
    {
      power_save = true;
//      u8g.sleepOn();
    }
  }

  switch(player_state)
  {
  case PS_Menu:
    player_class = question(class_names, trigger_up, trigger_down, trigger_a);
    if(player_class != -1)
    {
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
      player_state = PS_Game;
      newlevel();
    }
    break;
  case PS_Game:
    if(trigger_a) {
      if(player_clear == '>')
      {
        dungeon_level++;
        if(xp >= maxxp)
        {
          xptier++;
          maxxp += xptier;
          player_state = PS_Levelup;
        }
        else
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
//    if(trigger_y) {
//      u8g.sleepOn();
//    }
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
  case PS_Levelup:
    current_menu = question(stat_names, trigger_up, trigger_down, trigger_a);
    if(current_menu != -1)
    {
      switch(current_menu)
      {
      case 0:
        Strength += 1;
        break;
      case 1:
        Intelligence += 1;
        break;
      case 2:
        Agility += 1;
        break;
      }
      if(xp >= maxxp)
      {
        xptier++;
        maxxp += xptier;
        sprintf(view[LEVEL_V - 1], "%d (%d, %d, %d) %d/%d [%d]", hp, Strength, Intelligence, Agility, xp, maxxp, dungeon_level); // TODO: create function
      }
      else
      {
        player_state = PS_Game;
        newlevel();
      }
    }
    break;
  }

//  int n = 0;
//  u8g.firstPage();
//
//  do {
//    u8g.drawStr(0, n * 8 + 7, view[n]);
//    n++;
//  } while(u8g.nextPage()); 
  for(uint8_t y = 0; y < 8; y++)
  {
    i2c_start();
    i2c_send(SH1106_I2C_ADDRESS);
    i2c_send(SH1106_COMMAND);
    i2c_send(SH1106_COLUMN_LOW + 0);
    i2c_send(SH1106_COLUMN_HIGH + 0);
    i2c_send(0xB0 + y);
    i2c_stop();

    i2c_start();
    i2c_send(SH1106_I2C_ADDRESS);
    i2c_send(SH1106_DATA);
    put_string(view[y]);
    i2c_stop();
  }
  if(shake)
  {
    if(shake & 1)
      set_scroll(0);
    else
      set_scroll(2);
    shake--;
  }  
}
