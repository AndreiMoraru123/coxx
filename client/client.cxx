#include "client.hxx"

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
   * @param cmd The command to be send.
 * @return std::int32_t Error code indicating success (0) or failure (-1).
 */
std::int32_t Client::sendRequest(std::int64_t fd, const QueryArray cmd) const {
  std::uint32_t messageLength = 4;

  for (const std::string& s : cmd) {
    messageLength += 4 + s.size();
  }

  if (messageLength > MAX_MESSAGE_SIZE) {
    return -1;
  }

  std::vector<char> writeBuffer(4 + MAX_MESSAGE_SIZE);
  std::memcpy(writeBuffer.data(), &messageLength, 4);
  std::uint32_t n = cmd.size();
  std::memcpy(writeBuffer.data() + 4, &n, messageLength);

  std::size_t current = 8;
  for (const std::string& s : cmd) {
    std::uint32_t previous = static_cast<std::uint32_t>(s.size());
    std::memcpy(writeBuffer.data() + current, &previous, 4);
    std::memcpy(writeBuffer.data() + current + 4, s.data(), s.size());
    current += 4 + s.size();
  }

  std::string wbufStr(writeBuffer.begin(), writeBuffer.end());
  return socket.writeAll(fd, wbufStr, 4 + messageLength);
}

/**
 * @brief Reads a response from the server.
 *
 * This function reads a response from the server by first reading a 4-byte
 * header that indicates the length of the message, and then reading the message
 * itself.
 *
 * @param fd The file descriptor from which the response is read.
 * @return std::int32_t Error code indicating success (0) or failure (-1).
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
  auto response = Response::OK;
  if (messageLength < 4) {
    std::println("bad response");
    return -1;
  }

  auto responseValue =
      static_cast<std::underlying_type<Response>::type>(response);
  std::memcpy(&responseValue, readBuffer.data() + 4, 4);

  std::string_view responseView(readBuffer.data() + 8, messageLength - 4);
  std::println("Server says: [{}] {}", responseValue, responseView);
  return 0;
}

/**
 * @brief Runs the client with the provided queries to the server.
 *
 * This function sets up the client socket, binds it to a port, and sends a
 * list of queries to the server. It then reads the responses from the server
 * for each query.
 *
 * @param queryList The queries to send to the server.
 * @param port The port to run the client on.
 */
void Client::run(QueryArray queryList, std::int64_t port) {
  socket.setOptions();
  socket.configureConnection(port, CLIENT_NETADDR, "client");

  std::int32_t sendErr = sendRequest(socket.getFd(), queryList);
  if (sendErr) {
    std::cerr << "send request error" << std::endl;
  }

  std::int32_t readError = readResponse(socket.getFd());
  if (readError) {
    std::cerr << "read response error" << std::endl;
  }
}

/**
 * @brief Get the Socket object
 *
 * @return Socket&
 */
[[deprecated("only used in test")]]
Socket& Client::getSocket() {
  return socket;
}