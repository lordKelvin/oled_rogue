#pragma once
#include "config.h"
#include "cell.h"

enum MonsterClass {
  MC_Beast = 0,
  MC_Undead,
  MC_Flying
};

struct Monsters {
  unsigned char x, y;
  signed char dx, dy;
  enum CellTypes monster_clear;
  char monster_appearance;
  MonsterClass monster_class;
  unsigned stat_value;
  bool alive, ccw;
};

extern int monster_count;
extern Monsters monsters[MAX_MONSTERS];
extern int x, y, hp, maxhp, xp;
extern int Strength, Intelligence, Agility;
extern int dungeon_level;
extern uint8_t shake;
