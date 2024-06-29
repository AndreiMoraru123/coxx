#include <unistd.h>

#include <iostream>
#include <print>
#include <vector>

#include "socket.hxx"

class Server {
 public:
  Server() = default;
  void run();
  void doSomething(std::int64_t connfd);

 private:
  Socket socket;
};

void Server::doSomething(std::int64_t connfd) {
  std::vector<char> rbuf(64);
  ssize_t n = read(connfd, rbuf.data(), rbuf.size() - 1);
  if (n < 0) {
    std::cerr << "read() error" << std::endl;
    return;
  }

  std::println("Client says {}", std::string(rbuf.begin(), rbuf.begin() + n));

  std::string wbuf = "world";
  write(connfd, wbuf.c_str(), wbuf.length());
}

void Server::run() {
  socket.setOptions();
  socket.bindToPort(1234);

  if (listen(socket.getFd(), SOMAXCONN)) {
    throw ::std::runtime_error("Failed to listen");
  }

  while (true) {
    struct sockaddr_in client_addr = {};
    socklen_t socklen = sizeof(client_addr);
    std::int64_t connfd =
        accept(socket.getFd(), reinterpret_cast<struct sockaddr*>(&client_addr),
               &socklen);

    if (connfd == -1) {
      continue;
    }
    doSomething(connfd);
    close(connfd);
  }
}

auto main() -> int {
  Server server;
  server.run();
  return 0;
}