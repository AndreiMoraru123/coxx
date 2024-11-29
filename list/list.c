#include "list.h"

void listInit(DList *node) { node->prev = node->next = node; }

bool listEmpty(const DList *node) { return node->next == node; }

void listDetach(DList *node) {
  DList *prev = node->prev;
  DList *next = node->next;
  prev->next = next;
  next->prev = prev;
}

void listInsert(DList *target, DList *rookie) {
  DList *prev = target->prev;
  prev->next = rookie;
  rookie->prev = prev;
  rookie->next = target;
  target->prev = rookie;
}
