#pragma once
#include "config.h"

enum MonsterClass {
  MC_Beast = 0,
  MC_Undead,
  MC_Flying
};

struct Monsters {
  unsigned char x, y;
  signed char dx, dy;
  char monster_clear, monster_appearance;
  MonsterClass monster_class;
  unsigned stat_value;
  bool alive, ccw;
};

extern int monster_count;
extern Monsters monsters[MAX_MONSTERS];
extern char level[LEVEL_V][LEVEL_H];
extern char view[LEVEL_V][LEVEL_H + 1];
extern int x, y, hp, maxhp, xp;
extern int Strength, Intelligence, Agility;
extern int dungeon_level;
extern int shake;
