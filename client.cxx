#include <unistd.h>

#include <iostream>
#include <print>
#include <vector>

#include "socket.hxx"

class Client {
 public:
  Client() = default;
  void run();

 private:
  Socket socket;
};

void Client::run() {
  socket.setOptions();
  socket.bindToPort(1234, INADDR_LOOPBACK, "client");

  std::string msg = "hello";
  write(socket.getFd(), msg.c_str(), msg.length());

  std::vector<char> rbuf(64);
  ssize_t n = read(socket.getFd(), rbuf.data(), rbuf.size() - 1);
  if (n < 0) {
    std::cerr << "read() error" << std::endl;
    return;
  }

  std::println("Server says {}", std::string(rbuf.begin(), rbuf.begin() + n));
  close(socket.getFd());
}

auto main() -> int {
  Client client;
  client.run();
  return 0;
}