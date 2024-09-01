#include "client.hxx"

static std::int32_t deserialize(std::string_view data) {
  if (data.size() < 1) {
    std::println("bad response");
    return -1;
  }

  Serialize type = static_cast<Serialize>(data[0]);

  switch (type) {
    case Serialize::NIL:
      std::println("(nil)");
      return 1;
    case Serialize::ERR:
      if (data.size() < 1 + 8) {
        std::println("bad response");
        return -1;
      }
      {
        std::int32_t code = 0;
        std::uint32_t len = 0;
        std::memcpy(&code, &data[1], 4);
        std::memcpy(&len, &data[1 + 4], 4);
        if (data.size() < 1 + 8 + len) {
          std::println("bad response");
          return -1;
        }
        std::string_view dataView(&data[1 + 8], len);
        std::println("(err) {} {}", code, dataView);
        return 1 + 8 + len;
      }
    case Serialize::STR:
      if (data.size() < 1 + 4) {
        std::println("bad response");
        return -1;
      }
      {
        std::uint32_t len = 0;
        std::memcpy(&len, &data[1], 4);
        if (data.size() < 1 + 4 + len) {
          std::println("bad response");
          return -1;
        }
        std::string_view dataView(&data[1 + 4], len);
        std::println("(str) {}", dataView);
        return 1 + 4 + len;
      }
    case Serialize::INT:
      if (data.size() < 1 + 8) {
        std::println("bad response");
        return -1;
      }
      {
        std::int64_t val = 0;
        std::memcpy(&val, &data[1], 8);
        std::println("(int) {}", val);
        return 1 + 8;
      }
    case Serialize::ARR:
      if (data.size() < 1 + 4) {
        std::println("bad response");
        return -1;
      }
      {
        std::uint32_t len = 0;
        std::memcpy(&len, &data[1], 4);
        std::println("(arr) len={}", len);
        std::size_t arrayBytes = 1 + 4;
        for (std::uint32_t i = 0; i < len; ++i) {
          std::int32_t responseValue = deserialize(data.substr(arrayBytes));
          if (responseValue < 0) {
            return responseValue;
          }
          arrayBytes += static_cast<std::size_t>(responseValue);
        }
        std::println("(arr) end");
        return static_cast<std::int32_t>(arrayBytes);
      }
    default:
      std::println("bad response");
      return -1;
  }
}

/**
 * @brief Sends a request to the server.
 *
 * This function sends a request to the server by writing the length of the
 * message followed by the message itself to the specified file descriptor as
 the protocol demands:
 *
 * +-----+------+
 * | len | msg  |
 * +-----+------+
 *
 * @param fd The file descriptor to which the request is sent.
 * @param commands The command to be send.
 * @return Error code indicating success (0) or failure (-1).
 */
std::int32_t Client::sendRequest(std::int64_t fd,
                                 const CommandList commands) const {
  std::uint32_t messageLength = 4;

  for (const std::string& s : commands) {
    messageLength += 4 + s.size();
  }

  if (messageLength > MAX_MESSAGE_SIZE) {
    return -1;
  }

  std::vector<char> writeBuffer(4 + MAX_MESSAGE_SIZE);
  std::memcpy(writeBuffer.data(), &messageLength, 4);
  std::uint32_t n = commands.size();
  std::memcpy(writeBuffer.data() + 4, &n, messageLength);

  std::size_t current = 8;
  for (const std::string& s : commands) {
    std::uint32_t previous = static_cast<std::uint32_t>(s.size());
    std::memcpy(writeBuffer.data() + current, &previous, 4);
    std::memcpy(writeBuffer.data() + current + 4, s.data(), s.size());
    current += 4 + s.size();
  }

  std::string writeBufferString(writeBuffer.begin(), writeBuffer.end());
  return socket.writeAll(fd, writeBufferString, 4 + messageLength);
}

/**
 * @brief Reads a response from the server.
 *
 * This function reads a response from the server by first reading a 4-byte
 * header that indicates the length of the message, and then reading the message
 * itself.
 *
 * @param fd The file descriptor from which the response is read.
 * @return Error code indicating success (0) or failure (-1).
 */
std::int32_t Client::readResponse(std::int64_t fd) const {
  // 4 bytes header
  std::string header(4, '\0');
  std::vector<char> readBuffer(4 + MAX_MESSAGE_SIZE + 1);
  std::int32_t readError = socket.readFull(fd, header, 4);
  errno = 0;
  if (readError) {
    if (errno == 0) {
      std::println("EOF");
    } else {
      std::cerr << "read() error" << std::endl;
    }
    return readError;
  }

  // Copy the header back into readBuffer
  std::memcpy(readBuffer.data(), header.data(), 4);

  std::uint32_t messageLength = 0;
  std::memcpy(&messageLength, readBuffer.data(), 4);
  if (messageLength > MAX_MESSAGE_SIZE) {
    std::println("too long");
    return -1;
  }

  // Read the reply body
  std::string responseBody(messageLength, '\0');
  readError = socket.readFull(fd, responseBody, messageLength);
  if (readError) {
    std::cerr << "read() error" << std::endl;
    return readError;
  }

  // Copy the body back into readBuffer
  std::memcpy(readBuffer.data() + 4, responseBody.data(), messageLength);

  // Print the result
  std::int32_t responseValue =
      deserialize(std::string_view(readBuffer.data() + 4, messageLength));
  if (responseValue > 0 &&
      static_cast<std::uint32_t>(responseValue) != messageLength) {
    std::println("bad response");
    responseValue = -1;
  }

  return responseValue;
}

/**
 * @brief Runs the client with the provided queries to the server.
 *
 * This function sets up the client socket, binds it to a port, and sends a
 * list of queries to the server. It then reads the responses from the server
 * for each query.
 *
 * @param commands The queries to send to the server.
 * @param port The port to run the client on.
 */
void Client::run(CommandList commands, std::int64_t port) {
  socket.setOptions();
  socket.configureConnection(port, CLIENT_NETADDR, "client");

  std::int32_t sendErr = sendRequest(socket.getFd(), commands);
  if (sendErr) {
    std::cerr << "send request error" << std::endl;
  }

  std::int32_t readError = readResponse(socket.getFd());
  if (readError == -1) {
    std::cerr << "read response error" << std::endl;
  }
}