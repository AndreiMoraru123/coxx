#include <sys/epoll.h>
#include <unistd.h>

#include <array>
#include <cstring>
#include <iostream>
#include <memory>
#include <print>
#include <vector>

#include "common/conn.hxx"
#include "common/socket.hxx"

constexpr std::int64_t SERVER_PORT = 1234;
constexpr std::int64_t SERVER_NETADDR = 0;
constexpr std::int16_t SERVER_BACKLOG = SOMAXCONN;

class Server {
 public:
  /**
   * @brief Construct a new Server object.
   *
   */
  Server() = default;

  /**
   * @brief Runs the server event loop.
   *
   * This function first sets up the arguments for polling. The listening fd is
   * polled with the POLLIN flag. For the connection fd (connFd) the state of
   * the connection object (Conn) determines the poll flag. In this scenario,
   * the poll flag is either reading (POLLIN) or writing (POLLOUT), never both.
   * After `poll` returns, the server gets notified by which file descriptors
   * are ready for reading/writing and can process the connections in the
   * pollArgs vector.
   *
   * @param port The port to run the server on.
   */
  void run(std::int64_t port);

  /**
   * @brief Get the Socket object
   *
   * @return Socket&
   */
  Socket& getSocket();

 private:
  Socket socket;
};