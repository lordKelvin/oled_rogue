#pragma once
#include <stdint.h>

enum CellTypes
{
    CT_Wall = 0, // '#'
    CT_Floor = 1, // '.'
    CT_Exit = 2, // '>'
};

struct cell
{
  uint8_t seen: 1; // bool, the cell was seen
  uint8_t deadend: 1; // unused, bool, cell has only one entrance
  uint8_t empty: 1; // unused, bool, not a wall
  uint8_t secret: 1; // unused, bool, has a secret
  enum CellTypes type: 4; // object type (16) '#', '.', '>', ...
};

extern char celltype_to_char[];
