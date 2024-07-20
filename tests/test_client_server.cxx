#include "common.hxx"

constexpr std::int64_t TEST_PORT = 12345;

/**
 * @brief Handles one read() and one write() for a server connection.
 *
 * This function reads a message from the client connected through the file
 * descriptor @p connFd, stores it in a buffer, and then sends a response back
 * to the client using the provided write buffer @p wbuf.
 *
 * @param connFd The file descriptor for the connection to the client.
 * @param wbuf The message t obe send back to the client.
 * @return std::string The message received from the client. Returns an empty
 * string if the read operation fails.
 */
static std::string serverReadWrite(std::int64_t connFd, std::string wbuf) {
  std::vector<char> rbuf(TEST_BUFFER_SIZE);
  ssize_t n = read(connFd, rbuf.data(), rbuf.size() - 1);
  if (n < 0) {
    std::cerr << "read() error" << std::endl;
    return "";
  }

  std::string clientMsg(rbuf.begin(), rbuf.begin() + n);
  write(connFd, wbuf.c_str(), wbuf.length());
  return clientMsg;
}

/**
 * @brief Simulates a server run for testing.
 *
 * Initializes the server socket, binds it to the global common port and server
 * network address. For each accepted connection, it reads a message from a
 * single client, responds, and then closes the connection.
 *
 * This process is repeated for the maximum number of iterations until no more
 * connections are awaited.
 *
 * @param serverSocket A reference to the Socket object configured for the
 * server ops.
 * @param maxIterations The maximum number of client connections to accepts
 * before stopping. This is just to avoid infinite runs for the testing
 * scenario. In a real scenario, this could run indefinitely.
 * @return std::string The last message received from a client.
 * @throws std::runtime_error If the server fails to listen on the specified
 * port.
 */
static std::string run(Socket& serverSocket, std::int64_t maxIterations) {
  serverSocket.setOptions();
  serverSocket.bindToPort(TEST_PORT, TEST_SERVER_NETADDR, "server");

  if (listen(serverSocket.getFd(), TEST_BACKLOG)) {
    throw std::runtime_error("Failed to listen");
  }

  std::string lastClientMsg;

  for (int i = 0; i < maxIterations; ++i) {
    struct sockaddr_in clientAddr = {};
    socklen_t socklen = sizeof(clientAddr);
    std::int64_t connFd =
        accept(serverSocket.getFd(),
               reinterpret_cast<struct sockaddr*>(&clientAddr), &socklen);

    if (connFd == -1) {
      continue;
    }
    lastClientMsg = serverReadWrite(connFd, serverResponds);
    close(connFd);
  }

  return lastClientMsg;
}

/**
 * @brief Simulates client operations for testing.
 *
 * Initializes the client socket, connects it to the global common port and
 * client network address.
 *
 * After writing (sending) the message, it waits for a response from the server.
 *
 * This function is the core functionality tested in this file: the basic send
 * and receive of a single client socket in a controlled environment.
 *
 * @param clientSocket A reference to the Socket object configured for the
 * client ops.
 * @return std::string The response received from the server, if any.
 */
static std::string run(Socket& clientSocket) {
  clientSocket.setOptions();
  clientSocket.bindToPort(TEST_PORT, TEST_CLIENT_NETADDR, "client");

  write(clientSocket.getFd(), clientSays.c_str(), clientSays.length());

  std::vector<char> rbuf(64);
  ssize_t n = read(clientSocket.getFd(), rbuf.data(), rbuf.size() - 1);
  if (n < 0) {
    std::cerr << "read() error" << std::endl;
    return "";
  }
  std::string serverMsg(rbuf.begin(), rbuf.begin() + n);
  close(clientSocket.getFd());
  return serverMsg;
}

/**
 * @class ClientServerTest
 * @brief Test suite for client-server interaction.
 *
 * This test suite verifies the basic functionality of client-server
 * communication. It includes a test for sending and receiving messages between
 * a single client and a server.
 */
class ClientServerTest : public ::testing::Test {
 protected:
  Server server;
  Client client;
  std::thread serverThread;
  std::string clientMessage;
  std::string serverResponse;

  /**
   * @brief Set up the text fixture.
   *
   * Initializes the server in a separate thread. The reason for this is to give
   * the client the chance to respond. In a real scenario, the client and the
   * server would be different applications, with different runtimes.
   */
  void SetUp() override {
    serverThread = std::thread([this] {
      clientMessage = run(server.getSocket(), TEST_MAX_ITERATIONS);
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
 * @test Test case for a single client - server communication.
 *
 * Runs the client socket and asserts the received messages on both sides.
 *
 */
TEST_F(ClientServerTest, OneServerOneClient) {
  serverResponse = run(client.getSocket());

  EXPECT_EQ(clientMessage, "hello");
  EXPECT_EQ(serverResponse, "world");
}