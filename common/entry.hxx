#pragma once

#include <functional>
#include <memory>
#include <string>

#include "map/c/wrap.hxx"
#include "zset/zset.hxx"

enum class KeyType : std::uint8_t {
  STR = 0,
  ZSET = 1,
};

struct Entry {
  Node node;
  std::string key;
  std::uint32_t type = 0;
  std::string val;
  std::unique_ptr<ZSet> set = nullptr;
};

auto entryEquality(Node *lhs, Node *rhs) -> bool;
void scan(const Table &table, const std::function<void(Node *, void *)> &fn,
          void *arg);
