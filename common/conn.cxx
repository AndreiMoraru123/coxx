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

  std::uint8_t& requestData = *(readBuffer.data() + 4);

  // parse the request
  std::vector<std::string> command;
  if (0 != request.parse(requestData, messageLength, command)) {
    std::println("bad request");
    state = ConnectionState::END;
    return false;
  }

  // got one request, generate the response
  std::string output;
  request(command, output);

  // pack the response into the buffer
  if (4 + output.size() > MAX_MESSAGE_SIZE) {
    output.clear();
    out::err(output, static_cast<std::underlying_type_t<Error>>(Error::TOO_BIG),
             "response is too big");
  }

  std::uint32_t writeLength = static_cast<std::uint32_t>(output.size());
  std::memcpy(writeBuffer.data(), &writeLength, 4);
  std::memcpy(writeBuffer.data() + 4, output.data(), output.size());
  writeBufferSize = 4 + writeLength;

  // TODO: frequent memmove is efficient, need better handling
  std::size_t remainingSize = readBufferSize - 4 - messageLength;
  if (remainingSize) {
    std::memmove(readBuffer.data(), readBuffer.data() + 4 + messageLength,
                 remainingSize);
  }
  readBufferSize = remainingSize;

  // change state
  state = ConnectionState::RES;
  stateResponse();

  // continue to the outer loop if the request was fully processed
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
 * @return the file descriptor of the Connection.
 */
int Connection::getFd() const { return _fd; }

/**
 * @brief Get the connection state
 *
 * @return the state of the Connection.
 */
ConnectionState Connection::getState() const { return state; }

void Connection::io() {
  if (state == ConnectionState::REQ) {
    stateRequest();
  } else if (state == ConnectionState::RES) {
    stateResponse();
  } else [[unlikely]] {
    std::println("not expected");
    assert(0);
  }
}