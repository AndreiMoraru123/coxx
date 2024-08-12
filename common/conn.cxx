#include "conn.hxx"

Connection::~Connection() { close(_fd); }

bool Connection::tryOneRequest() {
  if (readBufferSize < 4) {
    // Not enough data in the buffer, will retry next iteration.
    return false;
  }

  std::uint32_t messageLength = 0;
  std::memcpy(&messageLength, readBuffer.data(), 4);
  if (messageLength > MAX_MESSAGE_SIZE) {
    std::println("too long");
    state = ConnectionState::END;
    return false;
  }

  if (4 + messageLength > readBufferSize) {
    return false;  // not enough data in the buffer;
  }

  auto response = Response::OK;
  std::uint32_t writeLength = 0;
  std::uint8_t& requestData = *(readBuffer.data() + 4);
  std::uint8_t& responseData = *(writeBuffer.data() + 4 + 4);

  std::int32_t err =
      request(requestData, messageLength, response, responseData, writeLength);

  if (err) {
    state = ConnectionState::END;
    return false;
  }

  writeLength += 4;
  std::memcpy(writeBuffer.data(), &writeLength, 4);
  auto responseValue =
      static_cast<std::underlying_type<Response>::type>(response);
  std::memcpy(writeBuffer.data() + 4, &responseValue, 4);
  writeBufferSize = 4 + writeLength;

  std::size_t remainingSize = readBufferSize - 4 - messageLength;
  if (remainingSize) {
    std::memmove(readBuffer.data(), readBuffer.data() + 4 + messageLength,
                 remainingSize);
  }
  readBufferSize = remainingSize;

  state = ConnectionState::RES;
  stateResponse();

  return (state == ConnectionState::REQ);
}

bool Connection::tryFlushBuffer() {
  ssize_t writtenBytes = 0;
  do {
    std::size_t remainingSize = writeBufferSize - writeBufferSent;
    writtenBytes =
        write(_fd, writeBuffer.data() + writeBufferSent, remainingSize);
  } while (writtenBytes < 0 && errno == EINTR);

  if (writtenBytes < 0 && errno == EAGAIN) {
    return false;  // stop
  }

  if (writtenBytes < 0) {
    std::cerr << "write() error" << std::endl;
    state = ConnectionState::END;
    return false;
  }

  writeBufferSent += static_cast<std::size_t>(writtenBytes);
  assert(writeBufferSent <= writeBufferSize);

  if (writeBufferSent == writeBufferSize) {
    state = ConnectionState::REQ;
    writeBufferSent = 0;
    writeBufferSize = 0;
    return false;
  }

  // still got some data in the write buffer, could try to write again
  return true;
}

bool Connection::tryFillBuffer() {
  assert(readBufferSize < readBuffer.capacity());
  ssize_t readBytes = 0;

  do {
    std::size_t availableCapacity = readBuffer.capacity() - readBufferSize;
    readBytes =
        read(_fd, readBuffer.data() + readBufferSize, availableCapacity);
  } while (readBytes < 0 && errno == EINTR);

  if (readBytes < 0 && errno == EAGAIN) {
    return false;  // stop
  }

  if (readBytes < 0) {
    std::cerr << "read() error" << std::endl;
    state = ConnectionState::END;
    return false;
  }

  if (readBytes == 0) {
    if (readBufferSize > 0) {
      std::println("unexpected EOF");
    } else {
      std::println("EOF");
    }
    state = ConnectionState::END;
    return false;
  }

  readBufferSize += static_cast<std::size_t>(readBytes);
  assert(readBufferSize <= readBuffer.capacity());

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