#include <map.hxx>

constexpr size_t RESIZING_WORK = 128;  // constant work
constexpr size_t MAX_LOAD_FACTOR = 8;

static void initialize(Table& table, size_t n) {
  assert(n > 0 && ((n - 1) & n) == 0);  // must be a power of 2
  table.table.resize(n);
  table.mask = n - 1;
  table.size = 0;
}

static void insert(Table& table, std::unique_ptr<Node>&& node) {
  std::size_t position = node->code & table.mask;  // slot index
  node->next = std::move(table.table[position]);   // prepend the list
  table.table[position] = std::move(node);
  table.size++;
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

  while (*from) {
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

static void helpResizing(Map& map) {
  size_t work = 0;
  while (work < RESIZING_WORK && map.table2.size > 0) {
    std::unique_ptr<Node>& from = map.table2.table[map.resizingPosition];

    if (!from) {
      map.resizingPosition++;
      continue;
    }

    insert(map.table1, std::move(detach(map.table2, from)));
    work++;
  }

  if (map.table2.size == 0 && !map.table2.table.empty()) {
    map.table2.table.clear();
    map.table2 = std::move(Table{});
  }
}

static void startResizing(Map& map) {
  assert(map.table2.table.empty());
  map.table2 = std::move(map.table1);
  initialize(map.table1, (map.table1.mask + 1) * 2);
  map.resizingPosition = 0;
}

std::unique_ptr<Node>* mapLookUp(
    Map& map, const std::unique_ptr<Node>& key,
    const std::function<bool(const std::unique_ptr<Node>&,
                             const std::unique_ptr<Node>&)>& equal) {
  helpResizing(map);
  std::unique_ptr<Node>* from = lookUp(map.table1, key, equal);
  from = from ? from : lookUp(map.table2, key, equal);
  return from ? from : nullptr;
}

void mapInsert(Map& map, std::unique_ptr<Node>&& node) {
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