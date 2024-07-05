#include <gtest/gtest.h>

#include <chrono>
#include <thread>

#include "../client.hxx"
#include "../server.hxx"

std::string serverDoSomething(std::int64_t connfd, std::string wbuf) {
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
  serverSocket.bindToPort(1234, 0, "server");

  if (listen(serverSocket.getFd(), SOMAXCONN)) {
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
    lastClientMsg = serverDoSomething(connfd, "world");
    close(connfd);
  }

  return lastClientMsg;
}

std::string runClient(Socket& clientSocket) {
  clientSocket.setOptions();
  clientSocket.bindToPort(1234, INADDR_LOOPBACK, "client");

  std::string msg = "hello";
  write(clientSocket.getFd(), msg.c_str(), msg.length());

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
    serverThread = std::thread(
        [this] { clientMessage = runServer(server.getSocket(), 1); });
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