
#include "entry.hxx"

auto entryEquality(CNode *lhs, CNode *rhs) -> bool {
  const Entry *le = containerOf(lhs, Entry, node);
  const Entry *re = containerOf(rhs, Entry, node);
  return le->key == re->key;
}

void entryDelete(const Entry &entry) {
  auto type = static_cast<KeyType>(entry.type);

  switch (type) {
  case KeyType::ZSET:
    zDispose(entry.set.get());
    break;
  case KeyType::STR:
    return;
  }
}

void scan(const CTable &table, const std::function<void(CNode *, void *)> &fn,
          void *arg) {
  if (table.size == 0)
    return;

  for (std::size_t i = 0; i < table.mask + 1; ++i) {
    CNode *node = table.table[i];
    while (node) {
      fn(node, arg);
      node = node->next;
    }
  }
}