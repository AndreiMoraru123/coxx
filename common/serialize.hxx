#pragma once

#include <cstdint>
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

enum class Serialize : std::uint8_t {
  NIL = 0,
  ERR = 1,
  STR = 2,
  INT = 4,
  ARR = 5,
};

namespace out {

void nil(std::string &out);
void str(std::string &out, const std::string &val);
void num(std::string &out, std::int64_t val);
void err(std::string &out, std::int32_t code, const std::string &msg);
void arr(std::string &out, std::uint32_t n);

}  // namespace out

void scan(CTable *table, const std::function<void(CNode *, void *)> &fn,
          void *arg);
void keyScan(CNode *node, void *arg);