#include <sys/epoll.h>
#include <unistd.h>

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
   * @brief Sets the file descriptor to nonblocking mode.
   *
   * @param fd the file descriptor to set into nonblocking mode
   */
  void makeNonBlocking(std::int64_t fd) const;

  /**
   * @brief Accepts a new connection and adds it to the fd2Conn vector.
   *
   *  This function accepts a new connection on the server's socket, makes it
   * non-blocking, creates a Conn object for it, and adds it to the vector that
   * maps the connections to their file descriptors.
   *
   * @param fd2Conn A vector of unique pointers to Conn objects, indexed by
   * their file descriptor.
   * @return std::int32_t Error integer indicating success (0) or failure (-1)
   */
  std::int32_t acceptNewConn(std::vector<std::unique_ptr<Conn>>& fd2Conn) const;

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