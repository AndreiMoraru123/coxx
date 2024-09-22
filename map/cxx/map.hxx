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

auto mapLookUp(Map &map, const Node &key,
               const std::function<bool(const std::unique_ptr<Node> &,
                                        const Node &)> &equal)
    -> std::unique_ptr<Node>;

void mapInsert(Map &map, std::unique_ptr<Node> &&node);

auto mapPop(Map &map, const Node &key,
            const std::function<bool(const std::unique_ptr<Node> &,
                                     const Node &)> &equal)
    -> std::unique_ptr<Node>;

auto mapSize(const Map &map) -> std::size_t;

void mapDestroy(Map &map);