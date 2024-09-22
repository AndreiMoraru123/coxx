#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct CNode {
  struct CNode *next;
  uint64_t code;
} CNode;

typedef struct CTable {
  CNode **table;
  size_t mask;
  size_t size;
} CTable;

typedef struct CMap {
  CTable table1;
  CTable table2;
  size_t resizingPosition;
} CMap;

void initNode(CNode *node);
void initTable(CTable *table);
void initMap(CMap *map);

CNode *CMapLookUp(CMap *map, CNode *key, bool (*eq)(CNode *, CNode *));
void CMapInsert(CMap *map, CNode *node);
CNode *CMapPop(CMap *map, CNode *key, bool (*eq)(CNode *, CNode *));
size_t CMapSize(const CMap *map);
void CMapDestroy(CMap *map);

#ifdef __cplusplus
}
#endif
