#include <table.hxx>

static void initialize(std::unique_ptr<Table> table, size_t n) {
  assert(n > 0 && ((n - 1) & n) == 0);  // must be a power of 2
  table->table.resize(n);
  table->mask = n - 1;
  table->size = 0;
}

static void insert(std::unique_ptr<Table> table, std::unique_ptr<Node> node) {
  std::size_t position = node->code & table->mask;  // slot index
  node->next = std::move(table->table[position]);   // prepend the list
  table->table[position] = std::move(node);
  table->size++;
}

static std::unique_ptr<Node>* lookUp(
    Table& table, const std::unique_ptr<Node>& key,
    const std::function<bool(const std::unique_ptr<Node>&,
                             const std::unique_ptr<Node>&)>& equal) {
  if (table.table.empty()) {
    return nullptr;
  }

  std::size_t position = key->code & table.mask;
  auto from = &table.table[position];  // incoming pointer to result

  while (&from) {
    if ((*from)->code == key->code && equal(*from, key)) {
      return from;
    }
    from = &(*from)->next;
  }

  return nullptr;
}

static std::unique_ptr<Node> detach(Table& table, std::unique_ptr<Node>& from) {
  if (!from) {
    return nullptr;
  }

  std::unique_ptr<Node> node = std::move(from);
  from = std::move(node->next);
  table.size--;
  return node;
}