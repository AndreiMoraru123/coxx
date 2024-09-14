#pragma once
#include <stdbool.h>
#include <stddef.h>  // NULL
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct AVLNode {
  uint32_t depth;  // subtree height
  uint32_t count;  // subtree size
  struct AVLNode *left;
  struct AVLNode *right;
  struct AVLNode *parent;
} AVLNode;

void init(AVLNode *node);
uint32_t depth(AVLNode *node);
uint32_t count(AVLNode *node);
void update(AVLNode *node);
AVLNode *rotateLeft(AVLNode *node);
AVLNode *rotateRight(AVLNode *node);
AVLNode *fixLeft(AVLNode *root);
AVLNode *fixRight(AVLNode *root);
AVLNode *fix(AVLNode *node);
AVLNode *del(AVLNode *node);

#ifdef __cplusplus
}
#endif