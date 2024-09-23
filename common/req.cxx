#include "req.hxx"
#include "common/entry.hxx"
#include "common/serialize.hxx"
#include "map/c/wrap.hxx"

#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <memory>
#include <utility>
#include <vector>

CommandMap Request::commandMap;

static void keyScan(const Node *node, void *arg) {
  std::string &output = *static_cast<std::string *>(arg);
  out::str(output, containerOf(node, Entry, node)->key);
}

static auto strToDouble(const std::string &s, std::double_t &output) {
  char *endPtr = nullptr;
  output = strtod(s.c_str(), &endPtr);
  return endPtr == s.c_str() + s.size() && !std::isnan(output);
}

static auto strToInt(const std::string &s, std::int64_t &output) {
  char *endPtr = nullptr;
  output = strtol(s.c_str(), &endPtr, 10);
  return endPtr == s.c_str() + s.size();
}

auto Request::expectZSet(std::string &output, std::string &s,
                         Entry **entry) const {
  Entry key;
  key.key.swap(s);
  key.node.code = stringHash(key.key);

  const auto *node = map::lookup(&commandMap.db, &key.node, &entryEquality);

  if (!node) {
    out::nil(output);
    return false;
  }

  *entry = containerOf(node, Entry, node);
  if ((*entry)->type != std::to_underlying(KeyType::ZSET)) {
    out::err(output, std::to_underlying(Error::TYPE), "expect zset");
    return false;
  }

  return true;
}

void Request::zadd(std::vector<std::string> &commandList,
                   std::string &output) const {
  std::double_t score = 0;
  if (!strToDouble(commandList[2], score)) {
    return out::err(output, std::to_underlying(Error::ARG), "expect fp number");
  }

  Entry key;
  key.key.swap(commandList[1]);
  key.node.code = stringHash(key.key);

  const auto *node = map::lookup(&commandMap.db, &key.node, &entryEquality);
  Entry *entry = nullptr;

  if (!node) {
    entry = new Entry();
    entry->key.swap(key.key);
    entry->node.code = key.node.code;
    entry->type = std::to_underlying(KeyType::ZSET);
    entry->set = std::make_unique<ZSet>();
    map::insert(&commandMap.db, &entry->node);
  } else {
    entry = containerOf(node, Entry, node);
    if (entry->type != std::to_underlying(KeyType::ZSET)) {
      return out::err(output, std::to_underlying(Error::TYPE), "expect zset");
    }
  }

  // add or update the tuple
  const std::string &name = commandList[3];
  auto added = zset::add(entry->set.get(), name.data(), name.size(), score);
  return out::num(output, static_cast<std::int64_t>(added));
}

void Request::zrem(std::vector<std::string> &commandList,
                   std::string &output) const {
  Entry *entry = nullptr;
  if (!expectZSet(output, commandList[1], &entry)) {
    return;
  }

  const std::string &name = commandList[2];
  auto *node = zset::pop(entry->set.get(), name.data(), name.size());
  if (node)
    zset::del(node);
  return out::num(output, node ? 1 : 0);
}

void Request::zscore(std::vector<std::string> &commandList,
                     std::string &output) const {

  Entry *entry = nullptr;
  if (!expectZSet(output, commandList[1], &entry)) {
    return;
  }

  const std::string &name = commandList[2];
  const auto *node = zset::lookup(entry->set.get(), name.data(), name.size());
  return node ? out::dbl(output, node->score) : out::nil(output);
}

void Request::zquery(std::vector<std::string> &commandList,
                     std::string &output) const {
  std::double_t score = 0;
  if (!strToDouble(commandList[2], score)) {
    return out::err(output, std::to_underlying(Error::ARG), "expect fp number");
  }

  const std::string &name = commandList[3];
  std::int64_t off = 0;
  std::int64_t limit = 0;

  if (!strToInt(commandList[4], off)) {
    return out::err(output, std::to_underlying(Error::ARG), "expect int");
  }
  if (!strToInt(commandList[5], limit)) {
    return out::err(output, std::to_underlying(Error::ARG), "expect int");
  }

  // get the set
  Entry *entry = nullptr;
  if (!expectZSet(output, commandList[1], &entry)) {
    if (output[0] == std::to_underlying(Serialize::NIL)) {
      output.clear();
      out::arr(output, 0);
    }
    return;
  }

  // look up the tuple
  if (limit <= 0) {
    return out::arr(output, 0);
  }
  auto *node = zset::query(entry->set.get(), score, name.data(), name.size());
  node = zOffset(node, off);

  // output
  auto arr = out::begin_arr(output);
  std::uint32_t n = 0;
  while (node && static_cast<std::int64_t>(n) < limit) {
    out::str(output, node->name);
    out::dbl(output, node->score);
    node = zOffset(node, +1);
    n += 2;
  }

  out::end_arr(output, arr, n);
}

auto Request::isCommand(const std::string &word,
                        const char *commandList) const {
  return 0 == strcasecmp(word.c_str(), commandList);
}

void Request::keys([[maybe_unused]] std::vector<std::string> &commandList,
                   std::string &output) const {
  out::arr(output, static_cast<std::uint32_t>(map::size(&commandMap.db)));
  scan(commandMap.db.table1, &keyScan, &output);
  scan(commandMap.db.table2, &keyScan, &output);
}

void Request::get(std::vector<std::string> &commandList,
                  std::string &output) const {
  Entry key;
  key.key.swap(commandList[1]);
  key.node.code = stringHash(key.key);

  const auto *node = map::lookup(&commandMap.db, &key.node, &entryEquality);

  if (!node) {
    return out::nil(output);
  }

  const std::string &value = containerOf(node, Entry, node)->val;
  out::str(output, value);
}

void Request::set(std::vector<std::string> &commandList,
                  std::string &output) const {
  Entry key;
  key.key.swap(commandList[1]);
  key.node.code = stringHash(key.key);

  const auto *node = map::lookup(&commandMap.db, &key.node, &entryEquality);

  if (node) {
    containerOf(node, Entry, node)->val.swap(commandList[2]);
  } else {
    auto entry = new Entry();
    entry->key.swap(key.key);
    entry->node.code = key.node.code;
    entry->val.swap(commandList[2]);
    map::insert(&commandMap.db, &entry->node);
  }

  return out::nil(output);
}

void Request::del(std::vector<std::string> &commandList,
                  std::string &output) const {
  Entry key;
  key.key.swap(commandList[1]);
  key.node.code = stringHash(key.key);

  const auto *node = map::pop(&commandMap.db, &key.node, &entryEquality);

  if (node) {
    delete containerOf(node, Entry, node);
  }

  return out::num(output, node ? 1 : 0);
}

auto Request::parse(std::uint8_t &requestData, std::size_t length,
                    std::vector<std::string> &outputData) -> std::uint32_t {
  if (length < 4) {
    return -1;
  }

  std::uint32_t count = 0;
  std::memcpy(&count, &requestData, 4);
  if (count > MAX_NUM_ARGS) {
    return -1;
  }

  std::size_t currentPosition = 4;
  while (count--) {
    if (currentPosition + 4 > length) {
      return -1;
    }

    std::uint32_t sizeOfData = 0;
    std::memcpy(&sizeOfData, &requestData + currentPosition, 4);

    if (currentPosition + 4 + sizeOfData > length) {
      return -1;
    }

    outputData.emplace_back(
        reinterpret_cast<char *>(&requestData + currentPosition + 4),
        sizeOfData);

    currentPosition += 4 + sizeOfData;
  }

  if (currentPosition != length) {
    return -1;
  }

  return 0;
}

void Request::operator()(std::vector<std::string> &commandList,
                         std::string &out) {
  if (commandList.size() == 1 && isCommand(commandList[0], "keys")) {
    keys(commandList, out);
  } else if (commandList.size() == 2 && isCommand(commandList[0], "get")) {
    get(commandList, out);
  } else if (commandList.size() == 3 && isCommand(commandList[0], "set")) {
    set(commandList, out);
  } else if (commandList.size() == 2 && isCommand(commandList[0], "del")) {
    del(commandList, out);
  } else if (commandList.size() == 4 && isCommand(commandList[0], "zadd")) {
    zadd(commandList, out);
  } else if (commandList.size() == 3 && isCommand(commandList[0], "zrem")) {
    zrem(commandList, out);
  } else if (commandList.size() == 3 && isCommand(commandList[0], "zscore")) {
    zscore(commandList, out);
  } else if (commandList.size() == 6 && isCommand(commandList[0], "zquery")) {
    zquery(commandList, out);
  } else {
    out::err(out, std::to_underlying(Error::UNKNOWN), "Unknown cmd");
  }
}