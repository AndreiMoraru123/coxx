#include "avl.h"

void init(AVLNode *node) {
  node->depth = 1;
  node->count = 1;
  node->left = NULL;
  node->right = NULL;
  node->parent = NULL;
}

static uint32_t max(uint32_t lhs, uint32_t rhs) {
  return lhs < rhs ? rhs : lhs;
}

uint32_t depth(AVLNode *node) { return node ? node->depth : 0; }

uint32_t count(AVLNode *node) { return node ? node->count : 0; }

void update(AVLNode *node) {
  node->depth = 1 + max(depth(node->left), depth(node->right));
  node->count = 1 + count(node->left) + count(node->right);
}

AVLNode *rotateLeft(AVLNode *node) {
  AVLNode *newNode = node->right;
  if (newNode->left) {
    newNode->left->parent = node;
  }
  node->right = newNode->left; // rotation
  newNode->left = node;        // rotation
  newNode->parent = node->parent;
  node->parent = newNode;
  update(node);
  update(newNode);
  return newNode;
}

AVLNode *rotateRight(AVLNode *node) {
  AVLNode *newNode = node->left;
  if (newNode->right) {
    newNode->right->parent = node;
  }
  node->left = newNode->right; // rotation
  newNode->right = node;       // rotation
  newNode->parent = node->parent;
  node->parent = newNode;
  update(node);
  update(newNode);
  return newNode;
}

AVLNode *fixLeft(AVLNode *root) {
  // the left subtree is too deep
  if (depth(root->left->left) < depth(root->left->right)) {
    root->left = rotateLeft(root->left); // rule 2
  }
  return rotateRight(root);
}

AVLNode *fixRight(AVLNode *root) {
  // the right subtree is too deep
  if (depth(root->right->right) < depth(root->right->left)) {
    root->right = rotateRight(root->right); // rule 2
  }
  return rotateLeft(root);
}

AVLNode *fix(AVLNode *node) {
  // fix imbalanced nodes and maintain invariants until the root is reached
  while (true) {
    update(node);
    uint32_t l = depth(node->left);
    uint32_t r = depth(node->right);
    AVLNode **from = NULL;
    AVLNode *p = node->parent;
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
    node = node->parent;
  }
}

AVLNode *del(AVLNode *node) {
  if (node->right == NULL) {
    // no right subtree, replace the node with the left subtree
    AVLNode *parent = node->parent;
    if (node->left) {
      // link the lft subtree to the parent
      node->left->parent = parent;
    }
    if (parent) {
      // attach the left subtree to the parent
      if (parent->left == node) {
        parent->left = node->left;
      } else {
        parent->right = node->left;
      }
      return fix(parent);
    } else {
      // removing root
      return node->left;
    }
  } else {
    // swap the node with its next sibling
    AVLNode *victim = node->right;
    while (victim->left) {
      victim = victim->left;
    }
    AVLNode *root = del(victim);

    *victim = *node;
    if (victim->left) {
      victim->left->parent = victim;
    }
    if (victim->right) {
      victim->right->parent = victim;
    }
    AVLNode *parent = node->parent;
    if (parent) {
      if (parent->left == node) {
        parent->left = victim;
      } else {
        parent->right = victim;
      }
      return root;
    } else {
      // removing root
      return victim;
    }
  }
}

AVLNode *offset(AVLNode *node, int64_t offset) {
  int64_t pos = 0; // relative to the starting node
  while (offset != pos) {
    if (pos < offset && pos + count(node->right) >= offset) {
      // the target is in the right subtree
      node = node->right;
      pos += count(node->left) + 1;
    } else if (pos > offset && pos - count(node->left) <= offset) {
      // the target is in the left subtree
      node = node->left;
      pos -= count(node->right) + 1;
    } else {
      // go to the parent
      AVLNode *parent = node->parent;
      if (!parent)
        return NULL; // out of range
      if (parent->right == node) {
        pos -= count(node->left) + 1;
      } else {
        pos += count(node->right) + 1;
      }
      node = parent;
    }
  }
  return node;
}