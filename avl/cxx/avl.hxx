#pragma once
#include <algorithm>
#include <boost/intrusive/avl_set.hpp>
#include <cstdint>
#include <memory>

using namespace boost::intrusive;

class AVLNode : public avl_set_base_hook<> {
 public:
  std::uint32_t depth;
  std::uint32_t count;
  AVLNode *parent;

  AVLNode() : depth(1), count(1), parent(nullptr) {}

  bool operator<(const AVLNode &other) const {
    return this->depth < other.depth;
  }
};

class RootComp {
 public:
  bool operator()(const AVLNode &a, const AVLNode &b) const { return false; }
  bool operator()(const AVLNode &a, const int b) const { return false; }
  bool operator()(const int a, const AVLNode &b) const { return false; }
};

class AVLTree {
 private:
  using AVLSet = avl_set<AVLNode, constant_time_size<true>>;

 public:
  AVLSet tree;
  void fix(AVLNode *node);
  auto root() -> AVLNode *;
  auto find_left(AVLNode *node) -> AVLNode *;
  auto find_right(AVLNode *node) -> AVLNode *;
  static void init(AVLNode *node);
  static auto depth(AVLNode *node) -> std::uint32_t;
  static auto count(AVLNode *node) -> std::uint32_t;
  void update(AVLNode *node);
  void insert(AVLNode *node);
  void erase(AVLNode *node);
};