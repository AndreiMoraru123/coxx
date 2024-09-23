
#include "entry.hxx"

auto entryEquality(Node *lhs, Node *rhs) -> bool {
  const auto *le = containerOf(lhs, Entry, node);
  const auto *re = containerOf(rhs, Entry, node);
  return le->key == re->key;
}

void entryDelete(const Entry &entry) {
  auto type = static_cast<KeyType>(entry.type);

  switch (type) {
  case KeyType::ZSET:
    zset::dispose(entry.set.get());
    break;
  case KeyType::STR:
    return;
  }
}

void scan(const Table &table, const std::function<void(Node *, void *)> &fn,
          void *arg) {
  if (table.size == 0)
    return;

  for (std::size_t i = 0; i < table.mask + 1; ++i) {
    auto *node = table.table[i];
    while (node) {
      fn(node, arg);
      node = node->next;
    }
  }
}