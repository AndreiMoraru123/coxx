#include <map.hxx>

constexpr size_t RESIZING_WORK = 128; // constant work
constexpr size_t MAX_LOAD_FACTOR = 8;

static void initialize(Table &table, size_t n) {
  assert(n > 0 && ((n - 1) & n) == 0); // must be a power of 2
  table.table.resize(n);
  table.mask = n - 1;
  table.size = 0;
}

static void insert(Table &table, std::unique_ptr<Node> &&node) {
  std::size_t position = node->code & table.mask; // slot index
  node->next = std::move(table.table[position]);  // prepend the list
  table.table[position] = std::move(node);
  table.size++;
}

static auto
lookUp(Table &table, const Node &key,
       const std::function<bool(const std::unique_ptr<Node> &, const Node &)>
           &equal) -> std::unique_ptr<Node> {
  if (table.table.empty()) {
    return nullptr;
  }

  std::size_t position = key.code & table.mask;
  auto from = &table.table[position]; // incoming pointer to result

  while (*from) {
    if ((*from)->code == key.code && equal(*from, key)) {
      return std::move(*from);
    }
    from = &(*from)->next;
  }

  return nullptr;
}

static auto detach(Table &table, std::unique_ptr<Node> &from)
    -> std::unique_ptr<Node> {
  if (!from) {
    return nullptr;
  }

  auto node = std::move(from);
  from = std::move(node->next);
  table.size--;
  return node;
}

static void helpResizing(Map &map) {
  size_t work = 0;
  while (work < RESIZING_WORK && map.table2.size > 0) {
    std::unique_ptr<Node> &from = map.table2.table[map.resizingPosition];

    if (!from) {
      map.resizingPosition++;
      continue;
    }

    insert(map.table1, detach(map.table2, from));
    work++;
  }

  if (map.table2.size == 0 && !map.table2.table.empty()) {
    map.table2.table.clear();
    map.table2 = Table{};
  }
}

static void startResizing(Map &map) {
  assert(map.table2.table.empty());
  map.table2 = std::move(map.table1);
  initialize(map.table1, (map.table1.mask + 1) * 2);
  map.resizingPosition = 0;
}

auto mapLookUp(Map &map, const Node &key,
               const std::function<bool(const std::unique_ptr<Node> &,
                                        const Node &)> &equal)
    -> std::unique_ptr<Node> {
  helpResizing(map);
  std::unique_ptr<Node> from = lookUp(map.table1, key, equal);
  from = from ? std::move(from) : lookUp(map.table2, key, equal);
  return from ? std::move(from) : nullptr;
}

void mapInsert(Map &map, std::unique_ptr<Node> &&node) {
  if (map.table1.table.empty()) {
    initialize(map.table1, 4);
  }

  insert(map.table1, std::move(node));

  if (map.table2.table.empty()) {
    // check if we need to resize
    size_t loadFactor = map.table1.size / (map.table1.mask + 1);
    if (loadFactor >= MAX_LOAD_FACTOR) {
      startResizing(map);
    }
  }
  helpResizing(map);
}

auto mapPop(Map &map, const Node &key,
            const std::function<bool(const std::unique_ptr<Node> &,
                                     const Node &)> &equal)
    -> std::unique_ptr<Node> {
  helpResizing(map);
  std::unique_ptr<Node> from = lookUp(map.table1, key, equal);
  if (from) {
    return detach(map.table1, from);
  }

  from = lookUp(map.table2, key, equal);
  if (from) {
    return detach(map.table2, from);
  }

  return nullptr;
}

auto mapSize(const Map &map) -> std::size_t {
  return map.table1.size + map.table2.size;
}

void mapDestroy(Map &map) {
  map.table1.table.clear();
  map.table2.table.clear();
  map = Map{};
}