#pragma once

#include <functional>
#include <memory>
#include <string>

#include "map/c/map.h"
#include "zset/zset.hxx"

enum class KeyType : std::uint8_t {
  STR = 0,
  ZSET = 1,
};

struct Entry {
  CNode node;
  std::string key;
  std::uint32_t type = 0;
  std::string val;
  std::unique_ptr<ZSet> set = nullptr;
};

auto entryEquality(CNode *lhs, CNode *rhs) -> bool;
void scan(const CTable &table, const std::function<void(CNode *, void *)> &fn,
          void *arg);
