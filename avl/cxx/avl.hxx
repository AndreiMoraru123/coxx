#pragma once
#include <algorithm>
#include <cstdint>
#include <memory>

struct AVL {
  std::uint32_t depth = 1;  // subtree height
  std::uint32_t count = 1;  // subtree size
  std::shared_ptr<AVL> left = nullptr;
  std::shared_ptr<AVL> right = nullptr;
  std::weak_ptr<AVL> parent;
};

std::uint32_t depth(const std::unique_ptr<AVL> &node);
std::uint32_t count(const std::unique_ptr<AVL> &node);
void update(std::unique_ptr<AVL> &node);
std::shared_ptr<AVL> rotateLeft(std::shared_ptr<AVL> &node);
std::shared_ptr<AVL> rotateRight(std::shared_ptr<AVL> &node);
std::shared_ptr<AVL> fixLeft(std::shared_ptr<AVL> &root);
std::shared_ptr<AVL> fixRight(std::shared_ptr<AVL> &root);
std::shared_ptr<AVL> fix(std::shared_ptr<AVL> &node);
std::shared_ptr<AVL> del(std::shared_ptr<AVL> &node);