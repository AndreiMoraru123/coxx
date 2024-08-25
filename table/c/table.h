#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct CNode {
  struct CNode* next;
  uint64_t code;
} CNode;

typedef struct CTable {
  CNode** table;
  size_t mask;
  size_t size;
} CTable;

typedef struct CMap {
  CTable table1;
  CTable table2;
  size_t resizingPosition;
} CMap;

void initNode(CNode* node);
void initTable(CTable* table);
void initMap(CMap* map);

#ifdef __cplusplus
}
#endif

// Node *mapLookUp(Map *map, Node *key, bool (*eq)(Node *, Node *));
// void mapInsert(Map *map, Node *node);
// Node mapPop(Map *map, Node *key);
// size_t mapSize(Map *map);
// void mapDestroy(Map *map);