#include "req.hxx"

std::map<std::string, std::string> Request::cmdMap;

bool Request::isCmd(const std::string& word, const char* cmd) {
  return 0 == strcasecmp(word.c_str(), cmd);
}

Response Request::get(const std::vector<std::string>& cmd,
                      std::uint8_t& response, std::uint32_t& responseLength) {
  if (!cmdMap.count(cmd[1])) {
    return Response::NX;
  }

  const std::string& value = cmdMap[cmd[1]];
  assert(value.size() <= K_MAX_MSG);

  std::memcpy(&response, value.data(), value.size());
  responseLength = static_cast<std::uint32_t>(value.size());
  return Response::OK;
}

Response Request::set(const std::vector<std::string>& cmd,
                      [[maybe_unused]] std::uint8_t& response,
                      [[maybe_unused]] std::uint32_t& responseLength) {
  cmdMap[cmd[1]] = cmd[2];
  return Response::OK;
}

Response Request::del(const std::vector<std::string>& cmd,
                      [[maybe_unused]] std::uint8_t& response,
                      [[maybe_unused]] std::uint32_t& responseLength) {
  cmdMap.erase(cmd[1]);
  return Response::OK;
}

std::uint32_t Request::parse(std::uint8_t& data, std::size_t len,
                             std::vector<std::string>& out) {
  if (len < 4) {
    return -1;
  }

  std::uint32_t n = 0;
  std::memcpy(&n, &data, 4);
  if (n > K_MAX_ARGS) {
    return -1;
  }

  std::size_t pos = 4;
  while (n--) {
    if (pos + 4 > len) {
      return -1;
    }

    std::uint32_t sizeOfData = 0;
    std::memcpy(&sizeOfData, &data + pos, 4);

    if (pos + 4 + sizeOfData > len) {
      return -1;
    }

    out.push_back(
        std::string(reinterpret_cast<char*>(&data + pos + 4), sizeOfData));

    pos += 4 + sizeOfData;
  }

  if (pos != len) {
    return -1;
  }

  return 0;
}

std::int32_t Request::operator()(std::uint8_t& request, std::uint32_t reqLen,
                                 Response& resCode, std::uint8_t& response,
                                 std::uint32_t& resLen) {
  std::vector<std::string> cmd;

  if (0 != parse(request, reqLen, cmd)) {
    std::println("bad request");
    return -1;
  }

  if (cmd.size() == 2 && isCmd(cmd[0], "get")) {
    resCode = get(cmd, response, resLen);
  } else if (cmd.size() == 3 && isCmd(cmd[0], "set")) {
    resCode = set(cmd, response, resLen);
  } else if (cmd.size() == 2 && isCmd(cmd[0], "del")) {
    resCode = del(cmd, response, resLen);
  } else {
    resCode = Response::ERR;
    const std::string msg = "Unknown cmd";
    std::strcpy(reinterpret_cast<char*>(&response), msg.c_str());
    resLen = std::strlen(msg.c_str());
    return 0;
  }
  return 0;
}