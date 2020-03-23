#include "monsters.h"
#include "cell.h"

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
    monsters[i].monster_clear = level_data[monsters[i].y][monsters[i].x].type;
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
    if(level_data[monsters[i].y][monsters[i].x].seen)
      set_char(monsters[i].x, monsters[i].y, monsters[i].monster_appearance);
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
  set_char(monsters[m].x, monsters[m].y, celltype_to_char[monsters[m].monster_clear]);
  monsters[m].alive = false;
  // show();
  set_char(x, y, '@');
  shake = 6;
}

void xchg(signed char &a, signed char &b)
{
  signed char tmp = a;
  a = b;
  b = tmp;
}

void monster_step(int m)
{
  Monsters &mon = monsters[m];
  if(mon.alive)
    monster_fight(m);
  if(!mon.alive)
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
      if(level_data[mon.y][mon.x].seen)
        set_char(mon.x, mon.y, celltype_to_char[mon.monster_clear]);
      mon.y += mon.dy;
      mon.x += mon.dx;
      mon.monster_clear = level_data[mon.y][mon.x].type;
      if(level_data[mon.y][mon.x].seen)
        set_char(mon.x, mon.y, mon.monster_appearance);
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
