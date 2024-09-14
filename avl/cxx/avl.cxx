#include "avl.hxx"

AVLNode *AVLTree::root() {
  if (tree.empty()) return nullptr;

  auto it = tree.find(0, RootComp());
  return (it != tree.end()) ? &(*it) : nullptr;
}

void AVLTree::init(AVLNode *node) {
  node->depth = 1;
  node->count = 1;
}

std::uint32_t AVLTree::depth(AVLNode *node) { return node ? node->depth : 0; }

std::uint32_t AVLTree::count(AVLNode *node) { return node ? node->count : 0; }

AVLNode *AVLTree::find_left(AVLNode *node) {
  if (!node) return nullptr;

  auto it = tree.iterator_to(*node);
  if (it == tree.begin()) return nullptr;  // no left node

  --it;
  return &(*it);
}

AVLNode *AVLTree::find_right(AVLNode *node) {
  if (!node) return nullptr;

  auto it = tree.iterator_to(*node);
  ++it;
  if (it == tree.end()) return nullptr;  // no right node

  return &(*it);
}

void AVLTree::update(AVLNode *node) {
  if (!node) return;

  std::uint32_t leftDepth = depth(find_left(node));
  std::uint32_t rightDepth = depth(find_right(node));
  node->depth = 1 + std::max(leftDepth, rightDepth);

  std::uint32_t leftCount = depth(find_left(node));
  std::uint32_t rightCount = depth(find_right(node));
  node->count = 1 + std::max(leftCount, rightCount);
}

void AVLTree::fix(AVLNode *node) {
  while (node) {
    update(node);
    node = node->parent;
  }
}

void AVLTree::insert(AVLNode *node) {
  auto [iter, inserted] = tree.insert_unique(*node);
  if (inserted) {
    auto parentIter = tree.iterator_to(*node);
    if (parentIter != tree.begin()) {
      --parentIter;
      AVLNode *parentNode = &(*parentIter);
      node->parent = parentNode;
    }
  }

  fix(node);
}

void AVLTree::erase(AVLNode *node) {
  tree.erase(tree.iterator_to(*node));
  AVLNode *parent = node->parent;
  if (parent) {
    fix(parent);
  }
}