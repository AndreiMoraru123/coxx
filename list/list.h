#pragma once
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct List {
  struct List *prev;
  struct List *next;
} DList;

void listInit(DList *node);
bool listEmpty(const DList *node);
void listDetach(DList *node);
void listInsert(DList *target, DList *rookie);

#ifdef __cplusplus
}
#endif
