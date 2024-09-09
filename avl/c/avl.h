#pragma once
#include <stdbool.h>
#include <stddef.h>  // NULL
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct C_AVL_Node {
  uint32_t depth;  // subtree height
  uint32_t count;  // subtree size
  struct C_AVL_Node *left;
  struct C_AVL_Node *right;
  struct C_AVL_Node *parent;
} C_AVL_Node;

void initAVL(C_AVL_Node *node);
uint32_t depth(C_AVL_Node *node);
uint32_t count(C_AVL_Node *node);
void update(C_AVL_Node *node);
C_AVL_Node *rotateLeft(C_AVL_Node *node);
C_AVL_Node *rotateRight(C_AVL_Node *node);
C_AVL_Node *fixLeft(C_AVL_Node *root);
C_AVL_Node *fixRight(C_AVL_Node *root);
C_AVL_Node *fix(C_AVL_Node *node);
C_AVL_Node *del(C_AVL_Node *node);

#ifdef __cplusplus
}
#endif