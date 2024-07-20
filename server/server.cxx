#include "server.hxx"

/**
 * @brief Sets the file descriptor to nonblocking mode.
 *
 * @param fd the file descriptor to set into nonblocking mode
 */
void Server::makeNonBlocking(std::int64_t fd) const {
  errno = 0;
  std::int64_t flags = fcntl(fd, F_GETFL, 0);
  if (errno) {
    std::cerr << "fcntl error" << std::endl;
  }

  flags |= O_NONBLOCK;

  fcntl(fd, F_SETFL, flags);
  if (errno) {
    std::cerr << "fcntl error" << std::endl;
  }
}

/**
 * @brief Adds a connection to the fd2Conn mapping vector.
 *
 * This function ensures that the fd2Conn vec is large enough to hold the
 * connection at the index corresponding to it's file descriptor. Resizes if
 * necessary, and then owns the connection.
 *
 * @param fd2Conn A vector of unique pointers to Conn objects, indexed by their
 * file descriptor.
 * @param conn A unique pointer to a Conn object to be added to the vector
 */
static void connPut(std::vector<std::unique_ptr<Conn>>& fd2Conn,
                    std::unique_ptr<Conn> conn) {
  if (fd2Conn.size() <= static_cast<std::size_t>(conn->getFd())) {
    fd2Conn.resize(conn->getFd() + 1);
  }
  fd2Conn[conn->getFd()] = std::move(conn);
}

/**
 * @brief Accepts a new connection and adds it to the fd2Conn vector.
 *
 *  This function accepts a new connection on the server's socket, makes it
 * non-blocking, creates a Conn object for it, and adds it to the vector that
 * maps the connections to their file descriptors.
 *
 * @param fd2Conn A vector of unique pointers to Conn objects, indexed by their
 * file descriptor.
 * @return std::int32_t Error integer indicating success (0) or failure (-1)
 */
std::int32_t Server::acceptNewConn(
    std::vector<std::unique_ptr<Conn>>& fd2Conn) const {
  struct sockaddr_in clientAddr = {};
  socklen_t socklen = sizeof(clientAddr);
  std::int64_t connFd =
      accept(socket.getFd(), reinterpret_cast<struct sockaddr*>(&clientAddr),
             &socklen);

  if (connFd < 0) {
    return -1;  // error
  }

  makeNonBlocking(connFd);
  auto conn = std::make_unique<Conn>(connFd, ConnState::REQ, 0);

  connPut(fd2Conn, std::move(conn));
  return 0;
}

/**
 * @brief Runs the server event loop.
 *
 * This function first sets up the arguments for polling. The listening fd is
 * polled with the POLLIN flag. For the connection fd (connFd) the state of the
 * connection object (Conn) determines the poll flag. In this scenario, the poll
 * flag is either reading (POLLIN) or writing (POLLOUT), never both. After
 * `poll` returns, the server gets notified by which file descriptors are ready
 * for reading/writing and can process the connections in the pollArgs vector.
 */
void Server::run() {
  socket.setOptions();
  socket.bindToPort(1234, 0, "server");

  if (listen(socket.getFd(), SOMAXCONN)) {
    throw std::runtime_error("Failed to listen");
  }

  // all client connections, keyed by fd
  std::vector<std::unique_ptr<Conn>> fd2Conn;
  makeNonBlocking(socket.getFd());

  // the event loop
  std::vector<pollfd> pollArgs;
  while (true) {
    // prepare the arguments of poll
    pollArgs.clear();
    // the listening fd is put first for convenience
    pollfd pfd = {socket.getFd(), POLLIN, 0};
    pollArgs.push_back(pfd);

    // connection fds
    for (auto& conn : fd2Conn) {
      if (!conn) {
        continue;
      }

      pollfd pfd = {};
      pfd.fd = conn->getFd();
      pfd.events = (conn->getState() == ConnState::REQ) ? POLLIN : POLLOUT;
      pfd.events = pfd.events | POLLERR;
      pollArgs.push_back(pfd);
    }

    // poll for active fds, timeout does not matter here
    if (poll(pollArgs.data(), static_cast<nfds_t>(pollArgs.size()), 1000) < 0) {
      std::cerr << "poll error" << std::endl;
    }

    // process active connections
    for (auto it = std::next(pollArgs.begin()); it != pollArgs.end(); ++it) {
      if (it->revents) {
        auto& conn = fd2Conn[it->fd];
        conn->io();
        if (conn->getState() == ConnState::END) {
          // client closed (normally or bad)
          fd2Conn[conn->getFd()].reset();  // destroy this connection
        }
      }
    }

    // try to accept a new connection if the listening fd is active
    if (pollArgs.front().revents) {
      acceptNewConn(fd2Conn);
    }
  }
}

/**
 * @brief Get the Socket object
 *
 * @return Socket&
 */
Socket& Server::getSocket() { return socket; }