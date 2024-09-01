
#include "entry.hxx"

bool entryEquality(CNode* lhs, CNode* rhs) {
  Entry* le = containerOf(lhs, Entry, node);
  Entry* re = containerOf(rhs, Entry, node);
  return le->key == re->key;
}

void scan(CTable& table, const std::function<void(CNode*, void*)>& fn,
          void* arg) {
  if (table.size == 0) return;

  for (std::size_t i = 0; i < table.mask + 1; ++i) {
    CNode* node = table.table[i];
    while (node) {
      fn(node, arg);
      node = node->next;
    }
  }
}