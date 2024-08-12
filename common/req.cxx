#include "req.hxx"

std::map<std::string, std::string> Request::commandMap;

bool Request::isCommand(const std::string& word, const char* commandList) {
  return 0 == strcasecmp(word.c_str(), commandList);
}

Response Request::get(const std::vector<std::string>& commandList,
                      std::uint8_t& responseValue,
                      std::uint32_t& responseLength) {
  if (!commandMap.count(commandList[1])) {
    return Response::NX;
  }

  const std::string& value = commandMap[commandList[1]];
  assert(value.size() <= MAX_MESSAGE_SIZE);

  std::memcpy(&responseValue, value.data(), value.size());
  responseLength = static_cast<std::uint32_t>(value.size());
  return Response::OK;
}

Response Request::set(const std::vector<std::string>& commandList,
                      [[maybe_unused]] std::uint8_t& responseValue,
                      [[maybe_unused]] std::uint32_t& responseLength) {
  commandMap[commandList[1]] = commandList[2];
  return Response::OK;
}

Response Request::del(const std::vector<std::string>& commandList,
                      [[maybe_unused]] std::uint8_t& responseValue,
                      [[maybe_unused]] std::uint32_t& responseLength) {
  commandMap.erase(commandList[1]);
  return Response::OK;
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

std::int32_t Request::operator()(std::uint8_t& requestData,
                                 std::uint32_t requestLength,
                                 Response& response,
                                 std::uint8_t& responseValue,
                                 std::uint32_t& responseLength) {
  std::vector<std::string> commandList;

  if (0 != parse(requestData, requestLength, commandList)) {
    std::println("bad request");
    return -1;
  }

  if (commandList.size() == 2 && isCommand(commandList[0], "get")) {
    response = get(commandList, responseValue, responseLength);
  } else if (commandList.size() == 3 && isCommand(commandList[0], "set")) {
    response = set(commandList, responseValue, responseLength);
  } else if (commandList.size() == 2 && isCommand(commandList[0], "del")) {
    response = del(commandList, responseValue, responseLength);
  } else {
    response = Response::ERR;
    const std::string msg = "Unknown command";
    std::strcpy(reinterpret_cast<char*>(&responseValue), msg.c_str());
    responseLength = std::strlen(msg.c_str());
    return 0;
  }
  return 0;
}