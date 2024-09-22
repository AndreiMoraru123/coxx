#include "map.h"

#include <assert.h>
#include <stdlib.h>

const size_t RESIZING_WORK = 128; // constant work
const size_t MAX_LOAD_FACTOR = 8;

void initNode(CNode *node) {
  node->next = NULL;
  node->code = 0;
}

void initTable(CTable *table) {
  table->table = NULL;
  table->mask = 0;
  table->size = 0;
}

void initMap(CMap *map) {
  initTable(&map->table1);
  initTable(&map->table2);
  map->resizingPosition = 0;
}

static void initialize(CTable *table, size_t n) {
  assert(n > 0 && ((n - 1) & n) == 0); // must be a power of 2
  table->table = (CNode **)calloc(sizeof(CNode *), n);
  table->mask = n - 1;
  table->size = 0;
}

static void insert(CTable *table, CNode *node) {
  size_t position = node->code & table->mask; // slot index
  CNode *next = table->table[position];       // prepend the list
  node->next = next;
  table->table[position] = node;
  table->size++;
}

static CNode **lookUp(CTable *table, CNode *key, bool (*eq)(CNode *, CNode *)) {
  if (!table->table) {
    return NULL;
  }

  size_t position = key->code & table->mask;
  CNode **from = &table->table[position];
  for (CNode *curr; (curr = *from) != NULL; from = &curr->next) {
    if (curr->code == key->code && eq(curr, key)) {
      return from;
    }
  }

  return NULL;
}

static CNode *detach(CTable *table, CNode **from) {
  CNode *node = *from;
  *from = node->next;
  table->size--;
  return node;
}

static void helpResizing(CMap *map) {
  size_t work = 0;
  while (work < RESIZING_WORK && map->table2.size > 0) {
    // scan for nodes from the second table and move them to the first
    CNode **from = &map->table2.table[map->resizingPosition];
    if (!*from) {
      map->resizingPosition++;
      continue;
    }

    insert(&map->table1, detach(&map->table2, from));
    work++;
  }

  if (map->table2.size == 0 && map->table2.table) {
    free(map->table2.table);
    initTable(&map->table2);
  }
}

static void startResizing(CMap *map) {
  assert(map->table2.table == NULL);
  map->table2 = map->table1;
  initialize(&map->table1, (map->table1.mask + 1) * 2);
  map->resizingPosition = 0;
}

CNode *CMapLookUp(CMap *map, CNode *key, bool (*eq)(CNode *, CNode *)) {
  helpResizing(map);
  CNode **from = lookUp(&map->table1, key, eq);
  from = from ? from : lookUp(&map->table2, key, eq);
  return from ? *from : NULL;
}

void CMapInsert(CMap *map, CNode *node) {
  if (!map->table1.table) {
    initialize(&map->table1, 4);
  }

  insert(&map->table1, node);

  if (!map->table2.table) {
    // check if we need to resize
    size_t loadFactor = map->table1.size / (map->table1.mask + 1);
    if (loadFactor >= MAX_LOAD_FACTOR) {
      startResizing(map);
    }
  }
  helpResizing(map);
}

CNode *CMapPop(CMap *map, CNode *key, bool (*eq)(CNode *, CNode *)) {
  helpResizing(map);
  CNode **from = lookUp(&map->table1, key, eq);
  if (from) {
    return detach(&map->table1, from);
  }

  from = lookUp(&map->table2, key, eq);
  if (from) {
    return detach(&map->table2, from);
  }

  return NULL;
}

size_t CMapSize(const CMap *map) { return map->table1.size + map->table2.size; }

void CMapDestroy(CMap *map) {
  free(map->table1.table);
  free(map->table2.table);
  initMap(map);
}
