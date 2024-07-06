#include <gtest/gtest.h>

#include <chrono>
#include <thread>

#include "../client.hxx"
#include "../server.hxx"

constexpr std::int64_t PORT = 1234;
constexpr std::int64_t SERVER_NETADDR = 0;
constexpr std::int64_t CLIENT_NETADDR = INADDR_LOOPBACK;
constexpr std::int16_t backlog = SOMAXCONN;

constexpr std::int64_t MAX_ITERATIONS = 1;

const std::string clientSays = "hello";
const std::string serverResponds = "world";

std::string serverReadWrite(std::int64_t connfd, std::string wbuf) {
  std::vector<char> rbuf(64);
  ssize_t n = read(connfd, rbuf.data(), rbuf.size() - 1);
  if (n < 0) {
    std::cerr << "read() error" << std::endl;
    return "";
  }

  std::string clientMsg(rbuf.begin(), rbuf.begin() + n);
  write(connfd, wbuf.c_str(), wbuf.length());
  return clientMsg;
}

std::string runServer(Socket& serverSocket, std::int64_t maxIterations) {
  serverSocket.setOptions();
  serverSocket.bindToPort(PORT, SERVER_NETADDR, "server");

  if (listen(serverSocket.getFd(), backlog)) {
    throw std::runtime_error("Failed to listen");
  }

  std::string lastClientMsg;

  for (int i = 0; i < maxIterations; ++i) {
    struct sockaddr_in client_addr = {};
    socklen_t socklen = sizeof(client_addr);
    std::int64_t connfd =
        accept(serverSocket.getFd(),
               reinterpret_cast<struct sockaddr*>(&client_addr), &socklen);

    if (connfd == -1) {
      continue;
    }
    lastClientMsg = serverReadWrite(connfd, serverResponds);
    close(connfd);
  }

  return lastClientMsg;
}

std::string runClient(Socket& clientSocket) {
  clientSocket.setOptions();
  clientSocket.bindToPort(PORT, CLIENT_NETADDR, "client");

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

class ClientServerTest : public ::testing::Test {
 protected:
  Server server;
  Client client;
  std::thread serverThread;
  std::string clientMessage;
  std::string serverResponse;

  void SetUp() override {
    serverThread = std::thread([this] {
      clientMessage = runServer(server.getSocket(), MAX_ITERATIONS);
    });
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }

  void TearDown() override {
    if (serverThread.joinable()) {
      serverThread.join();
    }
  }
};

TEST_F(ClientServerTest, OneServerOneClient) {
  serverResponse = runClient(client.getSocket());

  EXPECT_EQ(clientMessage, "hello");
  EXPECT_EQ(serverResponse, "world");
}