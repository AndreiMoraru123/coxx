#include <poll.h>
#include <unistd.h>

#include <cstring>
#include <iostream>
#include <memory>
#include <print>
#include <vector>

#include "common/conn.hxx"
#include "common/socket.hxx"

class Server {
 public:
  Server() = default;
  /**
   * @brief Sets the file descriptor to nonblocking mode.
   *
   */
  void makeNonBlocking(std::int64_t fd);
  std::int32_t acceptNewConn(std::vector<std::unique_ptr<Conn>>& fd2Conn);
  void run();
  Socket& getSocket();

 private:
  Socket socket;
};