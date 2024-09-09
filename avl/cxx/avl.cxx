#include "avl.hxx"

std::uint32_t depth(const std::shared_ptr<AVL> &node) {
  return node ? node->depth : 0;
}

std::uint32_t count(const std::shared_ptr<AVL> &node) {
  return node ? node->count : 0;
}

void update(std::shared_ptr<AVL> &node) {
  node->depth = 1 + std::max(depth(node->left), depth(node->right));
  node->count = 1 + count(node->left) + count(node->right);
}

std::shared_ptr<AVL> rotateLeft(std::shared_ptr<AVL> &node) {
  std::shared_ptr<AVL> newNode = node->right;
  if (newNode->left) {
    newNode->left->parent = node;
  }
  node->right = newNode->left;  // rotation
  newNode->left = node;         // rotation
  newNode->parent = node->parent;
  node->parent = newNode;
  update(node);
  update(newNode);
  return newNode;
}

std::shared_ptr<AVL> rotateRight(std::shared_ptr<AVL> &node) {
  std::shared_ptr<AVL> newNode = node->left;
  if (newNode->right) {
    newNode->right->parent = node;
  }
  node->left = newNode->right;  // rotation
  newNode->right = node;        // rotation
  newNode->parent = node->parent;
  node->parent = newNode;
  update(node);
  update(newNode);
  return newNode;
}

std::shared_ptr<AVL> fixLeft(std::shared_ptr<AVL> &root) {
  // the left subtree is too deep
  if (depth(root->left->left) < depth(root->left->right)) {
    root->left = rotateLeft(root->left);  // rule 2
  }
  return rotateRight(root);
}

std::shared_ptr<AVL> fixRight(std::shared_ptr<AVL> &root) {
  // the right subtree is too deep
  if (depth(root->right->right) < depth(root->right->left)) {
    root->right = rotateRight(root->right);  // rule 2
  }
  return rotateLeft(root);
}

std::shared_ptr<AVL> fix(std::shared_ptr<AVL> &node) {
  // fix imbalanced nodes and maintain invariants until the root is reached
  while (true) {
    update(node);
    uint32_t l = depth(node->left);
    uint32_t r = depth(node->right);
    std::shared_ptr<AVL> *from = nullptr;
    std::shared_ptr<AVL> p = node->parent.lock();
    if (p) {
      from = (p->left == node) ? &p->left : &p->right;
    }
    if (l == r + 2) {
      node = fixLeft(node);
    } else if (l + 2 == r) {
      node = fixRight(node);
    }
    if (!from) {
      return node;
    }
    *from = node;
    node = node->parent.lock();
  }
}

std::shared_ptr<AVL> del(std::shared_ptr<AVL> &node) {
  if (node->right == NULL) {
    // no right subtree, replace the node with the left subtree
    std::shared_ptr<AVL> parent = node->parent.lock();
    if (node->left) {
      // link the lft subtree to the parent
      node->left->parent = parent;
    }
    if (parent) {
      // attach the left subtree to the parent
      (parent->left == node ? parent->left : parent->right) = node->left;
      return fix(parent);
    } else {
      // removing root
      return node->left;
    }
  } else {
    // swap the node with its next sibling
    std::shared_ptr<AVL> victim = node->right;
    while (victim->left) {
      victim = victim->left;
    }
    std::shared_ptr<AVL> root = del(victim);

    *victim = *node;
    if (victim->left) {
      victim->left->parent = victim;
    }
    if (victim->right) {
      victim->right->parent = victim;
    }
    std::shared_ptr<AVL> parent = node->parent.lock();
    if (parent) {
      (parent->left == node ? parent->left : parent->right) = victim;
      return root;
    } else {
      // removing root
      return victim;
    }
  }
}