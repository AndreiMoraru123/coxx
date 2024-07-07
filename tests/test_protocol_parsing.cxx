#include <gtest/gtest.h>

#include <chrono>
#include <cstring>
#include <thread>

#include "common.hxx"

constexpr std::int64_t PORT = 12345;
constexpr std::uint8_t NUM_CLIENTS = 3;

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
      message = "read () error";
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

static std::string run(Socket& serverSocket, std::int64_t maxIterations,
                       std::uint8_t numClients) {
  serverSocket.setOptions();
  serverSocket.bindToPort(PORT, SERVER_NETADDR, "server");

  if (listen(serverSocket.getFd(), backlog)) {
    throw std::runtime_error("Failed to listen");
  }

  std::string lastClientMsg;

  for (int i = 0; i < maxIterations; ++i) {
    struct sockaddr_in client_addr = {};
    socklen_t socklen = sizeof(client_addr);
    std::int64_t connFd =
        accept(serverSocket.getFd(),
               reinterpret_cast<struct sockaddr*>(&client_addr), &socklen);

    if (connFd == -1) {
      continue;
    }

    for (int i = 0; i < numClients; ++i) {
      auto [err, msg] = oneRequest(serverSocket, connFd);
      if (err) {
        break;
      }
      lastClientMsg = msg;
    }
    close(connFd);
  }
  return lastClientMsg;
}

static std::pair<std::int32_t, std::string> run(Socket& clientSocket,
                                                std::uint8_t numClients) {
  clientSocket.setOptions();
  clientSocket.bindToPort(PORT, CLIENT_NETADDR, "client");

  std::pair<std::int32_t, std::string> response;
  for (int i = 0; i < numClients; ++i) {
    response = query(clientSocket, "hello");
  }

  close(clientSocket.getFd());
  return response;
}

class ClientServerTest : public ::testing::Test {
 protected:
  Server server;
  Client client;
  std::thread serverThread;
  std::mutex mutex;
  std::string lastClientMessage;

  /**
   * @brief Set up the text fixture.
   *
   * Initializes the server in a separate thread. The reason for this is to
   * give the client the chance to respond. In a real scenario, the client and
   * the server would be different applications, with different runtimes.
   */
  void SetUp() override {
    serverThread = std::thread([this] {
      lastClientMessage = run(server.getSocket(), MAX_ITERATIONS, NUM_CLIENTS);
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
TEST_F(ClientServerTest, ProtocolParsing) {
  auto lastServerMessage = run(client.getSocket(), NUM_CLIENTS);
  EXPECT_EQ(lastClientMessage, "hello");

  EXPECT_EQ(std::get<0>(lastServerMessage), 0);
  EXPECT_EQ(std::get<1>(lastServerMessage), "world");
}
