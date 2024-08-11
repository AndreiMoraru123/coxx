#include "conn.hxx"

Connection::~Connection() { close(_fd); }

bool Connection::tryOneRequest() {
  if (rbufSize < 4) {
    // Not enough data in the buffer, will retry next iteration.
    return false;
  }

  std::uint32_t len = 0;
  std::memcpy(&len, rbuf.data(), 4);
  if (len > K_MAX_MSG) {
    std::println("too long");
    state = ConnectionState::END;
    return false;
  }

  if (4 + len > rbufSize) {
    return false;  // not enough data in the buffer;
  }

  auto resCode = Response::OK;
  std::uint32_t wLen = 0;
  std::uint8_t& requestData = *(rbuf.data() + 4);
  std::uint8_t& responseData = *(wbuf.data() + 4 + 4);

  std::int32_t err = request(requestData, len, resCode, responseData, wLen);

  if (err) {
    state = ConnectionState::END;
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

  state = ConnectionState::RES;
  stateResponse();

  return (state == ConnectionState::REQ);
}

bool Connection::tryFlushBuffer() {
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
    state = ConnectionState::END;
    return false;
  }

  wbufSent += static_cast<std::size_t>(rv);
  assert(wbufSent <= wbufSize);

  if (wbufSent == wbufSize) {
    state = ConnectionState::REQ;
    wbufSent = 0;
    wbufSize = 0;
    return false;
  }

  // still got some data in the write buffer, could try to write again
  return true;
}

bool Connection::tryFillBuffer() {
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
    state = ConnectionState::END;
    return false;
  }

  if (rv == 0) {
    if (rbufSize > 0) {
      std::println("unexpected EOF");
    } else {
      std::println("EOF");
    }
    state = ConnectionState::END;
    return false;
  }

  rbufSize += static_cast<std::size_t>(rv);
  assert(rbufSize <= rbuf.capacity());

  while (tryOneRequest()) {
  }
  return (state == ConnectionState::REQ);
}

void Connection::stateRequest() {
  while (tryFillBuffer()) {
  }
}

void Connection::stateResponse() {
  while (tryFlushBuffer()) {
  }
}

/**
 * @brief Get the connection file descriptor
 *
 * @return int Returns the file descriptor of the Connection.
 */
int Connection::getFd() const { return _fd; }

/**
 * @brief Get the connection state
 *
 * @return int Returns the state of the Connection.
 */
ConnectionState Connection::getState() const { return state; }

void Connection::io() {
  if (state == ConnectionState::REQ) {
    stateRequest();
  } else if (state == ConnectionState::RES) {
    stateResponse();
  } else {
    std::println("not expected");
    assert(0);
  }
}