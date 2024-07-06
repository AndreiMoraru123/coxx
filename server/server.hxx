#include <unistd.h>

#include <cstring>
#include <iostream>
#include <print>
#include <vector>

#include "common/socket.hxx"

class Server {
 public:
  Server() = default;
  void run();
  void doSomething(std::int64_t connfd);
  std::int32_t oneRequest(std::int64_t connfd);
  Socket& getSocket();

 private:
  Socket socket;
};