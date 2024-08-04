#include "conn.hxx"

Conn::~Conn() { close(_fd); }

static bool isCommand(const std::string& word, const char* cmd) {
  return 0 == strcasecmp(word.c_str(), cmd);
}

static std::map<std::string, std::string> commandMap;

static Response doGet(const std::vector<std::string>& cmd,
                      std::uint8_t& response, std::uint32_t& responseLength) {
  if (!commandMap.count(cmd[1])) {
    return Response::NX;
  }

  const std::string& value = commandMap[cmd[1]];
  assert(value.size() <= K_MAX_MSG);

  std::memcpy(&response, value.data(), value.size());
  responseLength = static_cast<std::uint32_t>(value.size());
  return Response::OK;
}

static Response doSet(const std::vector<std::string>& cmd,
                      [[maybe_unused]] std::uint8_t& response,
                      [[maybe_unused]] std::uint32_t& responseLength) {
  commandMap[cmd[1]] = cmd[2];
  return Response::OK;
}

static Response doDel(const std::vector<std::string>& cmd,
                      [[maybe_unused]] std::uint8_t& response,
                      [[maybe_unused]] std::uint32_t& responseLength) {
  commandMap.erase(cmd[1]);
  return Response::OK;
}

static std::uint32_t parseRequest(std::uint8_t& data, std::size_t len,
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

static std::int32_t doRequest(std::uint8_t& request, std::uint32_t reqLen,
                              Response& resCode, std::uint8_t& response,
                              std::uint32_t& resLen) {
  std::vector<std::string> cmd;

  if (0 != parseRequest(request, reqLen, cmd)) {
    std::println("bad request");
    return -1;
  }

  if (cmd.size() == 2 && isCommand(cmd[0], "get")) {
    resCode = doGet(cmd, response, resLen);
  } else if (cmd.size() == 3 && isCommand(cmd[0], "set")) {
    resCode = doSet(cmd, response, resLen);
  } else if (cmd.size() == 2 && isCommand(cmd[0], "del")) {
    resCode = doDel(cmd, response, resLen);
  } else {
    resCode = Response::ERR;
    const std::string msg = "Unknown cmd";
    std::strcpy(reinterpret_cast<char*>(&response), msg.c_str());
    resLen = std::strlen(msg.c_str());
    return 0;
  }
  return 0;
}

bool Conn::tryOneRequest() {
  if (rbufSize < 4) {
    // Not enough data in the buffer, will retry next iteration.
    return false;
  }

  std::uint32_t len = 0;
  std::memcpy(&len, rbuf.data(), 4);
  if (len > K_MAX_MSG) {
    std::println("too long");
    state = ConnState::END;
    return false;
  }

  if (4 + len > rbufSize) {
    return false;  // not enough data in the buffer;
  }

  auto resCode = Response::OK;
  std::uint32_t wLen = 0;

  std::uint8_t& requestData = *(rbuf.data() + 4);
  std::uint8_t& responseData = *(wbuf.data() + 4 + 4);
  std::int32_t err = doRequest(requestData, len, resCode, responseData, wLen);

  if (err) {
    state = ConnState::END;
    return false;
  }

  wLen += 4;
  std::memcpy(wbuf.data(), &wLen, 4);
  auto resCodeValue =
      static_cast<std::underlying_type<Response>::type>(resCode);
  std::memcpy(wbuf.data() + 4, &resCodeValue, 4);
  wbufSize = 4 + wLen;

  std::size_t remain = rbufSize - 4 - len;
  if (remain) {
    std::memmove(rbuf.data(), rbuf.data() + 4 + len, remain);
  }
  rbufSize = remain;

  state = ConnState::RES;
  stateResponse();

  return (state == ConnState::REQ);
}

bool Conn::tryFlushBuffer() {
  ssize_t rv = 0;
  do {
    std::size_t remain = wbufSize - wbufSent;
    rv = write(_fd, wbuf.data() + wbufSent, remain);
  } while (rv < 0 && errno == EINTR);

  if (rv < 0 && errno == EAGAIN) {
    return false;  // stop
  }

  if (rv < 0) {
    std::cerr << "write() error" << std::endl;
    state = ConnState::END;
    return false;
  }

  wbufSent += static_cast<std::size_t>(rv);
  assert(wbufSent <= wbufSize);

  if (wbufSent == wbufSize) {
    state = ConnState::REQ;
    wbufSent = 0;
    wbufSize = 0;
    return false;
  }

  // still got some date in the write buffer, could try to write again
  return true;
}

bool Conn::tryFillBuffer() {
  assert(rbufSize < rbuf.capacity());
  ssize_t rv = 0;

  do {
    std::size_t cap = rbuf.capacity() - rbufSize;
    rv = read(_fd, rbuf.data() + rbufSize, cap);
  } while (rv < 0 && errno == EINTR);

  if (rv < 0 && errno == EAGAIN) {
    return false;  // stop
  }

  if (rv < 0) {
    std::cerr << "read() error" << std::endl;
    state = ConnState::END;
    return false;
  }

  if (rv == 0) {
    if (rbufSize > 0) {
      std::println("unexpected EOF");
    } else {
      std::println("EOF");
    }
    state = ConnState::END;
    return false;
  }

  rbufSize += static_cast<std::size_t>(rv);
  assert(rbufSize <= rbuf.capacity());

  while (tryOneRequest()) {
  }
  return (state == ConnState::REQ);
}

void Conn::stateRequest() {
  while (tryFillBuffer()) {
  }
}

void Conn::stateResponse() {
  while (tryFlushBuffer()) {
  }
}

/**
 * @brief Get the connection file descriptor
 *
 * @return int Returns the file descriptor of the Conn.
 */
int Conn::getFd() const { return _fd; }

/**
 * @brief Get the connection state
 *
 * @return int Returns the state of the Conn.
 */
ConnState Conn::getState() const { return state; }

void Conn::io() {
  if (state == ConnState::REQ) {
    stateRequest();
  } else if (state == ConnState::RES) {
    stateResponse();
  } else {
    std::println("not expected");
    assert(0);
  }
}