#include "req.hxx"

CommandMap Request::commandMap;

static bool entryEquality(CNode* lhs, CNode* rhs) {
  struct Entry* le = containerOf(lhs, Entry, node);
  struct Entry* re = containerOf(rhs, Entry, node);
  return le->key == re->key;
}

static std::uint64_t stringHash(std::string data, std::size_t length) {
  std::uint32_t hash = 0x811C9DC5;
  for (auto& letter : data) {
    hash = (hash + letter) * 0x01000193;
  }
  return hash;
}

bool Request::isCommand(const std::string& word, const char* commandList) {
  return 0 == strcasecmp(word.c_str(), commandList);
}

void Request::keys([[maybe_unused]] std::vector<std::string>& cmd,
                   std::string& output) {
  out::arr(output, static_cast<std::uint32_t>(CMapSize(&commandMap.db)));
  scan(commandMap.db.table1, &keyScan, &output);
  scan(commandMap.db.table2, &keyScan, &output);
}

void Request::get(std::vector<std::string>& commandList, std::string& output) {
  Entry key;
  key.key.swap(commandList[1]);
  key.node.code = stringHash(key.key, key.key.size());

  CNode* node = CMapLookUp(&commandMap.db, &key.node, &entryEquality);

  if (!node) {
    return out::nil(output);
  }

  const std::string& value = containerOf(node, Entry, node)->val;
  out::str(output, value);
}

void Request::set(std::vector<std::string>& commandList, std::string& output) {
  Entry key;
  key.key.swap(commandList[1]);
  key.node.code = stringHash(key.key, key.key.size());

  CNode* node = CMapLookUp(&commandMap.db, &key.node, &entryEquality);

  if (node) {
    containerOf(node, Entry, node)->val.swap(commandList[2]);
  } else {
    Entry* entry = new Entry();
    entry->key.swap(key.key);
    entry->node.code = key.node.code;
    entry->val.swap(commandList[2]);
    CMapInsert(&commandMap.db, &entry->node);
  }

  return out::nil(output);
}

void Request::del(std::vector<std::string>& commandList, std::string& output) {
  Entry key;
  key.key.swap(commandList[1]);
  key.node.code = stringHash(key.key, key.key.size());

  CNode* node = CMapPop(&commandMap.db, &key.node, &entryEquality);

  if (node) {
    delete containerOf(node, Entry, node);
  }

  return out::num(output, node ? 1 : 0);
}

std::uint32_t Request::parse(std::uint8_t& requestData, std::size_t length,
                             std::vector<std::string>& outputData) {
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

    outputData.push_back(
        std::string(reinterpret_cast<char*>(&requestData + currentPosition + 4),
                    sizeOfData));

    currentPosition += 4 + sizeOfData;
  }

  if (currentPosition != length) {
    return -1;
  }

  return 0;
}

void Request::operator()(std::vector<std::string>& commandList,
                         std::string& out) {
  if (commandList.size() == 1 && isCommand(commandList[0], "keys")) {
    keys(commandList, out);
  } else if (commandList.size() == 2 && isCommand(commandList[0], "get")) {
    get(commandList, out);
  } else if (commandList.size() == 3 && isCommand(commandList[0], "set")) {
    set(commandList, out);
  } else if (commandList.size() == 2 && isCommand(commandList[0], "del")) {
    del(commandList, out);
  } else {
    out::err(out, static_cast<std::underlying_type_t<Error>>(Error::UNKNOWN),
             "Unknown cmd");
  }
}