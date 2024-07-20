#include <gtest/gtest.h>

#include <chrono>
#include <cstring>
#include <thread>

#include "common.hxx"

constexpr std::int64_t TEST_PORT = 123456;
constexpr std::uint8_t TEST_NUM_QUERIES = 3;

/**
 * @brief Processes a single request from a client and sends back a response.
 *
 * This function implements the protocol by first reading a 4-byte header to
 * determine the length of the incoming message. It then reads the message of
 * that length. If the message length exceeds the maximum allowed (K_MAX_MSG),
 * it returns an error. After successfully reading the message, it builds a
 * response and sends it back to the client.
 *
 * +-----+------+-----+------+--------
 * | len | msg1 | len | msg2 | more...
 * +-----+------+-----+------+--------
 *
 * @param serverSocket A reference to the Socket object configured for the
 * server ops.
 * @param connFd The file descriptor for the connection to the client.
 * @return std::pair<std::int32_t, std::string> The error code (0 on success, -1
 * on failure) along with the message received from the client (on success) or
 * an error message (on failure).
 */
static std::pair<std::int32_t, std::string> oneRequest(Socket& serverSocket,
                                                       std::int64_t connFd) {
  std::vector<char> rbuf(4 + K_MAX_MSG + 1);
  errno = 0;

  std::string header(4, '\0');
  std::int32_t err = serverSocket.readFull(connFd, header, 4);
  if (err) {
    std::string message;
    if (errno == 0) {
      message = "EOF";
    } else {
      message = "read() error";
    }
    return {err, message};
  }

  // Copy the header back into rbuf
  std::memcpy(rbuf.data(), header.data(), 4);

  std::uint32_t len = 0;
  std::memcpy(&len, rbuf.data(), 4);
  if (len > K_MAX_MSG) {
    return {-1, "too long"};
  }

  // Read the request body
  std::string body(len, '\0');
  err = serverSocket.readFull(connFd, body, len);
  if (err) {
    return {err, "read() error"};
  }

  // Copy the body back into rbuf
  std::memcpy(rbuf.data() + 4, body.data(), len);

  // Print the null terminated string in the buffer
  rbuf[4 + len] = '\0';
  std::string clientMessage = &rbuf[4];

  // Reply using the same protocol
  std::vector<char> wbuf(4 + serverResponds.size());
  len = static_cast<std::uint32_t>(serverResponds.size());

  std::memcpy(wbuf.data(), &len, 4);
  std::memcpy(wbuf.data() + 4, serverResponds.data(), len);

  std::string wbufStr(wbuf.begin(), wbuf.end());
  return {serverSocket.writeAll(connFd, wbufStr, len + 4), clientMessage};
}

/**
 * @brief Sends a query to the server and waits for the response.
 *
 * Uses the same 4-byte protocol as the Server.
 *
 * @param clientSocket A reference to the Socket object configured for the
 * client ops.
 * @param text The message to send to the server.
 * @return std::pair<std::int32_t, std::string> The error code (0 on success, -1
 * on failure) along with the response received from the server (on success) or
 * an error message (on failure).
 */
static std::pair<std::int32_t, std::string> query(Socket& clientSocket,
                                                  std::string text) {
  std::int64_t fd = clientSocket.getFd();
  std::uint32_t len = static_cast<uint32_t>(text.size());

  if (len > K_MAX_MSG) {
    return {-1, "query too long"};
  }

  std::vector<char> wbuf(4 + K_MAX_MSG);
  std::memcpy(wbuf.data(), &len, 4);
  std::memcpy(wbuf.data() + 4, text.data(), len);

  std::string wbufStr(wbuf.begin(), wbuf.end());
  std::int32_t writeErr = clientSocket.writeAll(fd, wbufStr, 4 + len);
  if (writeErr) {
    return {writeErr, "write() error"};
  }

  // 4 bytes header
  std::string header(4, '\0');
  std::vector<char> rbuf(4 + K_MAX_MSG + 1);
  std::int32_t readErr = clientSocket.readFull(fd, header, 4);
  errno = 0;
  if (readErr) {
    std::string message;
    if (errno == 0) {
      message = "EOF";
    } else {
      message = "read() error";
    }
    return {readErr, message};
  }

  // Copy the header back into rbuf
  std::memcpy(rbuf.data(), header.data(), 4);

  std::memcpy(&len, rbuf.data(), 4);
  if (len > K_MAX_MSG) {
    return {-1, "too long"};
  }

  // Read the reply body
  std::string body(len, '\0');
  readErr = clientSocket.readFull(fd, body, len);
  if (readErr) {
    return {readErr, "read() error"};
  }

  // Copy the body back into rbuf
  std::memcpy(rbuf.data() + 4, body.data(), len);

  // Return server message
  rbuf[4 + len] = '\0';
  return {0, &rbuf[4]};
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
 * @return std::string The last client message the server processed.
 */
static std::vector<std::string> run(Socket& serverSocket,
                                    std::int64_t maxIterations,
                                    std::uint8_t numQueries) {
  serverSocket.setOptions();
  serverSocket.bindToPort(TEST_PORT, TEST_SERVER_NETADDR, "server");

  if (listen(serverSocket.getFd(), TEST_BACKLOG)) {
    throw std::runtime_error("Failed to listen");
  }

  std::vector<std::string> clientMessages;

  for (int i = 0; i < maxIterations; ++i) {
    struct sockaddr_in clientAddr = {};
    socklen_t socklen = sizeof(clientAddr);
    std::int64_t connFd =
        accept(serverSocket.getFd(),
               reinterpret_cast<struct sockaddr*>(&clientAddr), &socklen);

    if (connFd == -1) {
      continue;
    }

    for (int i = 0; i < numQueries; ++i) {
      auto [err, msg] = oneRequest(serverSocket, connFd);
      if (err) {
        break;
      }
      clientMessages.push_back(msg);
    }
    close(connFd);
  }
  return clientMessages;
}

/**
 * @brief Simulates client operations for testing.
 *
 * Initializes the server socket, binds it to the global common port and server
 * network address.
 *
 * Sends @p numQueries to the server and awaits a response back for each.
 *
 * This function is the core functionality tested in this file: the capability
 * of the server to handle multiple requests from a client.
 *
 * @param clientSocket A reference to the Socket object configured for the
 * client ops.
 * @param numQueries the number of requests (messages) the client will send.
 * @return std::pair<std::int32_t, std::string> The last error code along with
 * the last response received from the server for the last query.
 */
static std::vector<std::pair<std::int32_t, std::string>> run(
    Socket& clientSocket, std::uint8_t numQueries) {
  std::vector<std::pair<std::int32_t, std::string>> responses;

  clientSocket.setOptions();
  clientSocket.bindToPort(TEST_PORT, TEST_CLIENT_NETADDR, "client");

  std::pair<std::int32_t, std::string> response;
  for (int i = 0; i < numQueries; ++i) {
    responses.emplace_back(query(clientSocket, clientSays));
  }

  close(clientSocket.getFd());
  return responses;
}

class ClientServerTest : public ::testing::Test {
 protected:
  Server server;
  Client client;
  std::thread serverThread;
  std::vector<std::string> clientMessages;

  /**
   * @brief Set up the text fixture.
   *
   * Initializes the server in a separate thread. The reason for this is to
   * give the client the chance to respond. In a real scenario, the client and
   * the server would be different applications, with different runtimes.
   */
  void SetUp() override {
    serverThread = std::thread([this] {
      clientMessages =
          run(server.getSocket(), TEST_MAX_ITERATIONS, TEST_NUM_QUERIES);
    });
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }

  /**
   * @brief Tear down the text fixture.
   *
   * Ensures that the server thread is properly joined into the main thread.
   */
  void TearDown() override {
    if (serverThread.joinable()) {
      serverThread.join();
    }
  }
};

/**
 * @test Test case for a multiple requests from a client to a server via a
 * splitting protocol.
 *
 * Runs the client socket and asserts the last received messages on both sides.
 *
 */
TEST_F(ClientServerTest, ProtocolParsing) {
  auto serverResponses = run(client.getSocket(), TEST_NUM_QUERIES);

  for (const auto& msg : clientMessages) {
    EXPECT_EQ(msg, "hello");
  }

  for (const auto& [err, msg] : serverResponses) {
    EXPECT_EQ(err, 0);
    EXPECT_EQ(msg, "world");
  }
}
