#include "sh1106_i2c.h"
#include "config.h"
#include "monsters.h"
#include "cell.h"

// TODO: input string (name)
// TODO: save current state
// TODO: It's SSD1306, not sh1106

#define EEPROM_I2C_ADDRESS (0x50 << 1)

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
uint8_t shake = 0;
bool shake_up = false;

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
  PS_Inventory,
  PS_Lose,
  PS_Win, // TODO: Win scene
  PS_Levelup,
  PS_Nothing,
} player_state = PS_Menu;

int player_class;
enum CellTypes player_clear; // TODO: take this info from the map
int x, y;
int hp, maxhp, xp = 0, maxxp = 1, xptier = 1; // TODO: 16 bit variables + alter extern
int Strength, Intelligence, Agility;
int dungeon_level = 0;
/* Player: finish */

cell level_data[LEVEL_V][LEVEL_H];

void save()
{
  uint16_t addr = 0;
  uint8_t player_data[] = {
    1 // not dead
  , player_class
  , player_clear
  , x
  , y
  , hp
  , maxhp
  , xp
  , maxxp
  , xptier
  , Strength
  , Intelligence
  , Agility
  , dungeon_level
  , monster_count
  };
  eeprom_write(player_data, sizeof(player_data), addr);
  delay(10);
  addr += sizeof(player_data);
  for(int cy = 0; cy < LEVEL_V; cy++)
  {
    eeprom_write((uint8_t *)level_data[y], LEVEL_H, addr);
    delay(10);
    addr += LEVEL_H;
  }
  eeprom_write((uint8_t *)monsters, sizeof(monsters) * sizeof(*monsters), addr);
  delay(10);
  addr += sizeof(monsters) * sizeof(*monsters);
}

uint8_t load()
{
  uint16_t addr = 0;
  uint8_t player_data[15];
  eeprom_read(player_data, sizeof(player_data), 0);
  addr += sizeof(player_data);

  if(player_data[0] != 1)
    return 0;

  player_class  = player_data[1];
  player_clear  = player_data[2];
  x             = player_data[3];
  y             = player_data[4];
  hp            = player_data[5];
  maxhp         = player_data[6];
  xp            = player_data[7];
  maxxp         = player_data[8];
  xptier        = player_data[9];
  Strength      = player_data[10];
  Intelligence  = player_data[11];
  Agility       = player_data[12];
  dungeon_level = player_data[13];
  monster_count = player_data[14];

  for(int cy = 0; cy < LEVEL_V; cy++)
  {
    eeprom_read((uint8_t *)level_data[y], LEVEL_H, addr);
    addr += LEVEL_H;
  }
  eeprom_read((uint8_t *)monsters, sizeof(monsters) * sizeof(*monsters), addr);
  addr += sizeof(monsters) * sizeof(*monsters);

  return 1;
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
      level_data[r][c].seen = 0;
      if(r % 2 == 1 || c % 2 == 0)
        level_data[r][c].type = CT_Wall;
      else
        level_data[r][c].type = CT_Floor;
    }
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

        level_data[r * 2][c * 2 + 2].type = CT_Floor;
      }

      if(c != R[c] && random(2) == 0) // Should we connect this cell and its neighbour below?
      {
        R[L[c]] = R[c]; // Link L[c] to R[c]
        L[R[c]] = L[c];
        R[c] = c; // Link c to c
        L[c] = c;
      } else {
        level_data[r * 2 + 1][c * 2 + 1].type = CT_Floor;
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

      level_data[(LEVEL_V / 2 - 1) * 2][c * 2 + 2].type = CT_Floor;
    }

    R[L[c]] = R[c]; // Link L[c] to R[c]
    L[R[c]] = L[c];
    R[c] = L[c] = c; // Link c to c
  }
}

void newlevel(void)
{
  clear_screen();
  eller();
  if(monster_count < MAX_MONSTERS - 1)
    monster_count++;
  x = random(LEVEL_H / 2) * 2 + 1;
  y = random(LEVEL_V / 2) * 2;
  level_data[random(LEVEL_V / 2) * 2][random(LEVEL_H / 2) * 2 + 1].type = CT_Exit;
  player_clear = level_data[y][x].type;
  show();
  set_char(x, y, '@');
  generate_monsters();
}

#define DEBUG

#ifdef DEBUG
  #define PRINT_VAR(x) (Serial.print(#x " = "), Serial.println(x))
  #define PRINT_EXPR(n, x) (Serial.print(n " = "), Serial.println(x))
#else
  #define PRINT_VAR(x)
  #define PRINT_EXPR(n, x)
#endif

void setup(void)
{
  #ifdef DEBUG
  Serial.begin(9600);
  #endif
  PRINT_VAR(F_CPU);
  PRINT_EXPR("f_scl", F_CPU / (16 + 2 * TWBR * (int)(pow(4, (TWSR & 3)))));
  PRINT_VAR(TWBR);
  PRINT_VAR(TWSR);
  PRINT_VAR(TWPS0);
  PRINT_VAR(TWPS1);
  PRINT_VAR(SDA);
  PRINT_VAR(SCL);
  PRINT_VAR(TWEN);
  PRINT_VAR(TWIE);
  PRINT_VAR(TWEA);
  PRINT_VAR(NOT_A_PIN);
  PRINT_VAR(NOT_ON_TIMER);
  PRINT_EXPR("sizeof(cell)", sizeof(cell));
  PRINT_EXPR("SDA_timer", digitalPinToTimer(SDA));
  PRINT_EXPR("SDA_bit", digitalPinToBitMask(SDA));
  PRINT_EXPR("SDA_port", digitalPinToPort(SDA));
  PRINT_EXPR("SCL_timer", digitalPinToTimer(SCL));
  PRINT_EXPR("SCL_bit", digitalPinToBitMask(SCL));
  PRINT_EXPR("SCL_port", digitalPinToPort(SCL));
  PRINT_EXPR("&TWSR", _SFR_ADDR(TWSR));
  PRINT_EXPR("sizeof(TWSR)", sizeof(TWSR));



  randomSeed(analogRead(0) ^ millis()); // TODO: micros() !!!
  DDRD &= ~0b11111100; PORTD |= 0b11111100; // set digital pin 2-7 as INPUT_PULLUP
  DDRB &= ~0b00000011; PORTB |= 0b00000011; // set digital pin 8-9 as INPUT_PULLUP

  delay(100);
  i2c_init();
  delay(100);

  PRINT_VAR(TWBR);

  display_init();
  clear_screen();
  timeout = millis();
  if(load() == 1)
    player_state = PS_Game;
}

bool is_empty(int x, int y)
{
  if(x < 0 || y < 0 || x >= LEVEL_H || y >= LEVEL_V)
    return false;

  return (level_data[y][x].type == CT_Floor || level_data[y][x].type == CT_Exit); // TODO: use flags
}

void print_stats()
{
  set_printf(0, LEVEL_V - 1, "%d (%d, %d, %d) %d/%d [%d]", hp, Strength, Intelligence, Agility, xp, maxxp, dungeon_level);
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
      if(level_data[py][px].seen)
        continue;
      if(px == x - 2 || px == x + 2 || py == y - 2 || py == y + 2)
      {
        int tmpx = px;
        if(px < x) tmpx++;
        if(px > x) tmpx--;
        if(is_empty(tmpx, tmpy))
        {
          set_char(px, py, celltype_to_char[level_data[py][px].type]);
          level_data[py][px].seen = 1;
        }
      }
      else
      {
        set_char(px, py, celltype_to_char[level_data[py][px].type]);
        level_data[py][px].seen = 1;
      }
    }
  }
  print_stats();
}

void move(int dx, int dy)
{
  if(is_empty(x + dx, y + dy))
  {
    set_char(x, y, celltype_to_char[player_clear]);
    x += dx;
    y += dy;
    player_clear = level_data[y][x].type;
    show();
    set_char(x, y, '@');
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
      set_string(1, count, options[count]);
  }
  if(sub_item && current_item > 0)
    current_item--;
  if(add_item && current_item < count - 1)
    current_item++;
  for(int i = 0; i < count; i++)
    set_char(0, i, (current_item == i ? '>' : ' '));
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
    if(trigger_left || trigger_up || trigger_right || trigger_down || trigger_a || trigger_b || trigger_x || trigger_y)
    {
      if(power_save)
      {
        power_save = false;
        display_toggle(true);
      }
      timeout = current_time;
    }
    else if(current_time - timeout > SCREENSAVER_TIMEOUT_MS)
    {
      power_save = true;
      display_toggle(false);
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
      if(player_clear == CT_Exit)
      {
        dungeon_level++;
        if(xp >= maxxp)
        {
          xptier++;
          maxxp += xptier;
          player_state = PS_Levelup;
          clear_screen();
          print_stats();
        }
        else
        {
          newlevel();
          save();
        }
      }
    }
    if(trigger_b) {
      for(int y = 0; y < LEVEL_V - 1; y++)
      {
        for(int x = 0; x < LEVEL_H; x++)
        {
          level_data[y][x].seen = 1;
          set_char(x, y, celltype_to_char[level_data[y][x].type]);
        }
      }
    }
    if(trigger_x) {
      dungeon_level++;
      newlevel();
    }
    if(trigger_y) {
      display_toggle(false);
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
    clear_screen();
    set_printf(0, 0, "You lost your life");
    set_printf(0, 1, " on %d-%s level of", dungeon_level, (dungeon_level == 1 ? "st" : (dungeon_level == 2 ? "nd" : (dungeon_level == 3 ? "rd" : "th"))));
    set_printf(0, 2, " gloomy dungeon");
    set_printf(0, 3, " scoring %d", xp);
    set_printf(0, 4, " expirience points.");
    print_stats();
    player_state = PS_Nothing;
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
        print_stats();
      }
      else
      {
        player_state = PS_Game;
        newlevel();
        save();
      }
    }
    break;
  case PS_Nothing:
    break;
  }

  if(shake)
  {
    if(millis() & 0x20)
    {
      if(shake_up)
      {
        shake_up = false;
        shake--;
      }
      set_scroll(0);
    }
    else
    {
      if(!shake_up)
        shake_up = true;
      set_scroll(2);
    }
  }
}
