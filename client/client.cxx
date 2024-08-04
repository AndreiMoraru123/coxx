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
  // std::uint32_t len = static_cast<std::uint32_t>(text.size());
  std::uint32_t len = 4;

  for (const std::string& s : cmd) {
    len += 4 + s.size();
  }

  if (len > K_MAX_MSG) {
    return -1;
  }

  std::vector<char> wbuf(4 + K_MAX_MSG);
  std::memcpy(wbuf.data(), &len, 4);
  std::uint32_t n = cmd.size();
  std::memcpy(wbuf.data() + 4, &n, len);

  std::size_t current = 8;
  for (const std::string& s : cmd) {
    std::uint32_t previous = static_cast<std::uint32_t>(s.size());
    std::memcpy(wbuf.data() + current, &previous, 4);
    std::memcpy(wbuf.data() + current + 4, s.data(), s.size());
    current += 4 + s.size();
  }

  std::string wbufStr(wbuf.begin(), wbuf.end());
  return socket.writeAll(fd, wbufStr, 4 + len);
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
  std::vector<char> rbuf(4 + K_MAX_MSG + 1);
  std::int32_t readErr = socket.readFull(fd, header, 4);
  errno = 0;
  if (readErr) {
    if (errno == 0) {
      std::println("EOF");
    } else {
      std::cerr << "read() error" << std::endl;
    }
    return readErr;
  }

  // Copy the header back into rbuf
  std::memcpy(rbuf.data(), header.data(), 4);

  std::uint32_t len = 0;
  std::memcpy(&len, rbuf.data(), 4);
  if (len > K_MAX_MSG) {
    std::println("too long");
    return -1;
  }

  // Read the reply body
  std::string body(len, '\0');
  readErr = socket.readFull(fd, body, len);
  if (readErr) {
    std::cerr << "read() error" << std::endl;
    return readErr;
  }

  // Copy the body back into rbuf
  std::memcpy(rbuf.data() + 4, body.data(), len);

  // Print the result
  auto resCode = Response::OK;
  if (len < 4) {
    std::println("bad response");
    return -1;
  }

  auto resCodeValue =
      static_cast<std::underlying_type<Response>::type>(resCode);
  std::memcpy(&resCodeValue, rbuf.data() + 4, 4);

  std::string_view responseView(rbuf.data() + 8, len - 4);
  std::println("Server says: [{}] {}", resCodeValue, responseView);
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
  socket.bindToPort(port, CLIENT_NETADDR, "client");

  std::int32_t sendErr = sendRequest(socket.getFd(), queryList);
  if (sendErr) {
    std::cerr << "send request error" << std::endl;
  }

  std::int32_t readErr = readResponse(socket.getFd());
  if (readErr) {
    std::cerr << "read response error" << std::endl;
  }
}

/**
 * @brief Get the Socket object
 *
 * @return Socket&
 */
Socket& Client::getSocket() { return socket; }