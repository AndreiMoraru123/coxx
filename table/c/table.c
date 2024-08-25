#include "table.h"

#include <assert.h>
#include <stdlib.h>

void initNode(CNode* node) {
  node->next = NULL;
  node->code = 0;
}

void initTable(CTable* table) {
  table->table = NULL;
  table->mask = 0;
  table->size = 0;
}

void initMap(CMap* map) {
  initTable(&map->table1);
  initTable(&map->table2);
  map->resizingPosition = 0;
}

static void init(CTable* table, size_t n) {
  assert(n > 0 && ((n - 1) & n) == 0);  // must be a power of 2
  table->table = (CNode**)calloc(sizeof(CNode*), n);
  table->mask = n - 1;
  table->size = 0;
}

static void insert(CTable* table, CNode* node) {
  size_t position = node->code & table->mask;  // slot index
  CNode* next = table->table[position];        // prepend the list
  node->next = next;
  table->table[position] = node;
  table->size++;
}

static CNode** lookUp(CTable* table, CNode* key, bool (*eq)(CNode*, CNode*)) {
  if (!table->table) {
    return NULL;
  }

  size_t position = key->code & table->mask;
  CNode** from = &table->table[position];
  for (CNode* curr; (curr = *from) != NULL; from = &curr->next) {
    if (curr->code == key->code && eq(curr, key)) {
      return from;
    }
  }

  return NULL;
}

static CNode* detach(CTable* table, CNode** from) {
  CNode* node = *from;
  *from = node->next;
  table->size--;
  return node;
}
