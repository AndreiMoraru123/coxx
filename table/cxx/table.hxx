#pragma once
#include <cassert>
#include <cstdint>
#include <functional>
#include <memory>
#include <vector>

struct Node {
  std::unique_ptr<Node> next = nullptr;
  std::uint64_t code = 0;
};

struct Table {
  std::vector<std::unique_ptr<Node>> table;
  std::size_t mask = 0;
  std::size_t size = 0;
};

struct Map {
  Table table1;
  Table table2;
  std::size_t resizingPosition = 0;
};