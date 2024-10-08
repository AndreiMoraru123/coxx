#include "common.hxx"

constexpr std::int64_t TEST_PORT = 123456;
constexpr std::int64_t TEST_NUM_QUERIES = 3;

using ErrCodeMsgTuple = std::pair<std::int32_t, std::string>;

/**
 * @brief Processes a single request from a client and sends back a response.
 *
 * This function implements the protocol by first reading a 4-byte header to
 * determine the length of the incoming message. It then reads the message of
 * that length. If the message length exceeds the maximum allowed
 * (MAX_MESSAGE_SIZE), it returns an error. After successfully reading the
 * message, it builds a response and sends it back to the client.
 *
 * +-----+------+-----+------+--------
 * | len | msg1 | len | msg2 | more...
 * +-----+------+-----+------+--------
 *
 * @param serverSocket A reference to the Socket object configured for the
 * server ops.
 * @param connectionFileDescriptor The file descriptor for the connection to the
 * client.
 * @return The error code (0 on success, -1 on failure) along with the message
 * received from the client (on success) or an error message (on failure).
 */
static auto oneRequest(const Socket &serverSocket,
                       std::int64_t connectionFileDescriptor)
    -> ErrCodeMsgTuple {
  std::vector<char> readBuffer(4 + MAX_MESSAGE_SIZE + 1);
  errno = 0;

  std::string header(4, '\0');
  std::int32_t err = serverSocket.readFull(connectionFileDescriptor, header, 4);
  if (err) {
    std::string message;
    if (errno == 0) {
      message = "EOF";
    } else {
      message = "read() error";
    }
    return {err, message};
  }

  // Copy the header back into readBuffer
  std::memcpy(readBuffer.data(), header.data(), 4);

  std::uint32_t len = 0;
  std::memcpy(&len, readBuffer.data(), 4);
  if (len > MAX_MESSAGE_SIZE) {
    return {-1, "too long"};
  }

  // Read the request body
  std::string body(len, '\0');
  err = serverSocket.readFull(connectionFileDescriptor, body, len);
  if (err) {
    return {err, "read() error"};
  }

  // Copy the body back into readBuffer
  std::memcpy(readBuffer.data() + 4, body.data(), len);

  // Print the null terminated string in the buffer
  readBuffer[4 + len] = '\0';
  std::string clientMessage = &readBuffer[4];

  // Reply using the same protocol
  std::vector<char> writeBuffer(4 + serverResponds.size());
  len = static_cast<std::uint32_t>(serverResponds.size());

  std::memcpy(writeBuffer.data(), &len, 4);
  std::memcpy(writeBuffer.data() + 4, serverResponds.data(), len);

  std::string writeBufferString(writeBuffer.begin(), writeBuffer.end());
  return {serverSocket.writeAll(connectionFileDescriptor, writeBufferString,
                                len + 4),
          clientMessage};
}

/**
 * @brief Sends a query to the server and waits for the response.
 *
 * Uses the same 4-byte protocol as the Server.
 *
 * @param clientSocket A reference to the Socket object configured for the
 * client ops.
 * @param text The message to send to the server.
 * @return The error code (0 on success, -1 on failure) along with the response
 * received from the server (on success) or an error message (on failure).
 */
static auto query(const Socket &clientSocket, std::string_view text)
    -> ErrCodeMsgTuple {
  std::int64_t fd = clientSocket.getFd();
  auto len = static_cast<uint32_t>(text.size());

  if (len > MAX_MESSAGE_SIZE) {
    return {-1, "query too long"};
  }

  std::vector<char> writeBuffer(4 + MAX_MESSAGE_SIZE);
  std::memcpy(writeBuffer.data(), &len, 4);
  std::memcpy(writeBuffer.data() + 4, text.data(), len);

  std::string writeBufferString(writeBuffer.begin(), writeBuffer.end());
  if (std::int32_t writeError =
          clientSocket.writeAll(fd, writeBufferString, 4 + len)) {
    return {writeError, "write() error"};
  }

  // 4 bytes header
  std::string header(4, '\0');
  std::vector<char> readBuffer(4 + MAX_MESSAGE_SIZE + 1);
  std::int32_t readError = clientSocket.readFull(fd, header, 4);
  errno = 0;
  if (readError) {
    std::string message;
    if (errno == 0) {
      message = "EOF";
    } else {
      message = "read() error";
    }
    return {readError, message};
  }

  // Copy the header back into readBuffer
  std::memcpy(readBuffer.data(), header.data(), 4);

  std::memcpy(&len, readBuffer.data(), 4);
  if (len > MAX_MESSAGE_SIZE) {
    return {-1, "too long"};
  }

  // Read the reply body
  std::string body(len, '\0');
  readError = clientSocket.readFull(fd, body, len);
  if (readError) {
    return {readError, "read() error"};
  }

  // Copy the body back into readBuffer
  std::memcpy(readBuffer.data() + 4, body.data(), len);

  // Return server message
  readBuffer[4 + len] = '\0';
  return {0, &readBuffer[4]};
}

/**
 * @brief Simulates a server run for testing.
 *
 * Initializes the server socket, binds it to the global common port and server
 * network address. For each accepted connection, it reads a number of @p
 * numQueries from the client, responds to every one of them, and then closes
 * the connection.
 *
 * @param serverSocket A reference to the Socket object configured for the
 * server ops.
 * @param maxIterations The maximum number of client connections to accepts
 * before stopping. This is just to avoid infinite runs for the testing
 * scenario. In a real scenario, this could run indefinitely.
 * @param numQueries the number of requests (messages) the server will process.
 * @return The last client message the server processed.
 */
static auto run(const Socket &serverSocket, std::int64_t maxIterations,
                std::int64_t numQueries) -> std::vector<std::string> {
  serverSocket.setOptions();
  serverSocket.configureConnection(TEST_PORT, TEST_SERVER_NETADDR, "server");

  if (listen(serverSocket.getFd(), TEST_BACKLOG)) {
    throw std::runtime_error("Failed to listen");
  }

  std::vector<std::string> clientMessages;

  std::ranges::for_each(std::views::iota(0, maxIterations), [&](auto) {
    sockaddr_in clientAddress = {};
    socklen_t socketLength = sizeof(clientAddress);
    std::int64_t connectionFileDescriptor =
        accept(serverSocket.getFd(),
               reinterpret_cast<sockaddr *>(&clientAddress), &socketLength);

    if (connectionFileDescriptor == -1) {
      return false; // This will come back to bite me
    }

    std::ranges::for_each(std::views::iota(0, numQueries), [&](auto) {
      auto [err, msg] = oneRequest(serverSocket, connectionFileDescriptor);
      if (err) {
        return false;
      }
      clientMessages.push_back(msg);
      return true;
    });

    close(connectionFileDescriptor);
    return true;
  });
  return clientMessages;
}
/**
 * @brief Simulates client operations for testing.
 *
 * Initializes the server socket, binds it to the global common port and
 * server network address.
 *
 * Sends @p numQueries to the server and awaits a response back for each.
 *
 * This function is the core functionality tested in this file: the capability
 * of the server to handle multiple requests from a client.
 *
 * @param clientSocket A reference to the Socket object configured for the
 * client ops.
 * @param numQueries the number of requests (messages) the client will send.
 * @return The last error code along with
 * the last response received from the server for the last query.
 */
static auto run(const Socket &clientSocket, std::uint8_t numQueries)
    -> std::vector<ErrCodeMsgTuple> {
  std::vector<ErrCodeMsgTuple> responses;

  clientSocket.setOptions();
  clientSocket.configureConnection(TEST_PORT, TEST_CLIENT_NETADDR, "client");

  responses.reserve(numQueries);
  for (int i = 0; i < numQueries; ++i) {
    responses.emplace_back(query(clientSocket, clientSays));
  }

  return responses;
}

class ProtocolParsingTest : public ::testing::Test {
protected:
  Socket serverSocket;
  Socket clientSocket;
  std::jthread serverThread;
  std::vector<std::string> clientMessages;

  /**
   * @brief Set up the text fixture.
   *
   * Initializes the server in a separate thread. The reason for this is to
   * give the client the chance to respond. In a real scenario, the client and
   * the server would be different applications, with different runtimes.
   */
  void SetUp() override {
    serverThread = std::jthread([this] {
      clientMessages = run(serverSocket, TEST_MAX_ITERATIONS, TEST_NUM_QUERIES);
    });
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
};

/**
 * @test Test case for a multiple requests from a client to a server via a
 * splitting protocol.
 *
 * Runs the client socket and asserts the last received messages on both
 * sides.
 *
 */
TEST_F(ProtocolParsingTest, ProtocolParsing) {
  auto serverResponses = run(clientSocket, TEST_NUM_QUERIES);

  for (const auto &msg : clientMessages) {
    EXPECT_EQ(msg, "hello");
  }

  for (const auto &[err, msg] : serverResponses) {
    EXPECT_EQ(err, 0);
    EXPECT_EQ(msg, "world");
  }
}