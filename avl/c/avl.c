#include "avl.h"

void initAVL(C_AVL_Node *node) {
  node->depth = 1;
  node->count = 1;
  node->left = NULL;
  node->right = NULL;
  node->parent = NULL;
}

static uint32_t max(uint32_t lhs, uint32_t rhs) {
  return lhs < rhs ? rhs : lhs;
}

uint32_t depth(C_AVL_Node *node) { return node ? node->depth : 0; }

uint32_t count(C_AVL_Node *node) { return node ? node->count : 0; }

void update(C_AVL_Node *node) {
  node->depth = 1 + max(depth(node->left), depth(node->right));
  node->count = 1 + count(node->left) + count(node->right);
}

C_AVL_Node *rotateLeft(C_AVL_Node *node) {
  C_AVL_Node *newNode = node->right;
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

C_AVL_Node *rotateRight(C_AVL_Node *node) {
  C_AVL_Node *newNode = node->left;
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

C_AVL_Node *fixLeft(C_AVL_Node *root) {
  // the left subtree is too deep
  if (depth(root->left->left) < depth(root->left->right)) {
    root->left = rotateLeft(root->left);  // rule 2
  }
  return rotateRight(root);
}

C_AVL_Node *fixRight(C_AVL_Node *root) {
  // the right subtree is too deep
  if (depth(root->right->right) < depth(root->right->left)) {
    root->right = rotateRight(root->right);  // rule 2
  }
  return rotateLeft(root);
}

C_AVL_Node *fix(C_AVL_Node *node) {
  // fix imbalanced nodes and maintain invariants until the root is reached
  while (true) {
    update(node);
    uint32_t l = depth(node->left);
    uint32_t r = depth(node->right);
    C_AVL_Node **from = NULL;
    C_AVL_Node *p = node->parent;
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

C_AVL_Node *del(C_AVL_Node *node) {
  if (node->right == NULL) {
    // no right subtree, replace the node with the left subtree
    C_AVL_Node *parent = node->parent;
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
    C_AVL_Node *victim = node->right;
    while (victim->left) {
      victim = victim->left;
    }
    C_AVL_Node *root = del(victim);

    *victim = *node;
    if (victim->left) {
      victim->left->parent = victim;
    }
    if (victim->right) {
      victim->right->parent = victim;
    }
    C_AVL_Node *parent = node->parent;
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