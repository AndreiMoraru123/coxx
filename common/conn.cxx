#include "conn.hxx"

Conn::~Conn() { close(_fd); }

bool Conn::tryOneRequest() {
  if (rbufSize < 4) {
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

  std::string clientMessage(rbuf.begin() + 4, rbuf.begin() + 4 + len);
  std::println("Client says {}", clientMessage);

  std::memcpy(wbuf.data(), &len, 4);
  std::memcpy(wbuf.data() + 4, rbuf.data() + 4, len);
  wbufSize = 4 + len;

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
int Conn::getFd() { return _fd; }

/**
 * @brief Get the connection state
 *
 * @return int Returns the state of the Conn.
 */
ConnState Conn::getState() { return state; }

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