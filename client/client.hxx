#include <unistd.h>

#include <array>
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
  std::int32_t sendRequest(std::int64_t fd, std::string text);
  std::int32_t readResponse(std::int64_t fd);
  Socket& getSocket();

 private:
  Socket socket;
  static constexpr std::size_t k_max_msg = 4096;
};
