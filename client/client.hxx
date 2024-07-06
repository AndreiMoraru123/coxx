#include <unistd.h>

#include <cstring>
#include <iostream>
#include <print>
#include <string>
#include <vector>

#include "common/socket.hxx"

class Client {
 public:
  Client() = default;
  void run();
  std::int32_t query(std::int64_t fd, std::string text);
  Socket& getSocket();

 private:
  Socket socket;
};
