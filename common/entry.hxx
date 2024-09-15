#pragma once

#include <functional>
#include <string>

#include "map/c/map.h"

#define containerOf(ptr, type, member)                   \
  ({                                                     \
    const decltype(((type *)0)->member) *__mptr = (ptr); \
    (type *)((char *)__mptr - offsetof(type, member));   \
  })

struct Entry {
  CNode node;
  std::string key;
  std::string val;
};

auto entryEquality(CNode *lhs, CNode *rhs) -> bool;
void scan(CTable &table, const std::function<void(CNode *, void *)> &fn,
          void *arg);
