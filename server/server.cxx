#include "server.hxx"

constexpr std::int64_t MAX_EVENTS = 32;

static void epollCtlAdd(std::int64_t epFd, std::int64_t fd,
                        std::uint32_t events) {
  epoll_event ev;
  ev.events = events;
  ev.data.fd = fd;
  if (epoll_ctl(epFd, EPOLL_CTL_ADD, fd, &ev) == -1) {
    std::cerr << "epoll_ctl() error" << std::endl;
  }
}

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
  sockaddr_in clientAddr = {};
  socklen_t socklen = sizeof(clientAddr);
  std::int64_t connFd = accept(
      socket.getFd(), reinterpret_cast<sockaddr*>(&clientAddr), &socklen);

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
 *
 * @param port The port to run the server on.
 */
void Server::run(std::int64_t port) {
  socket.setOptions();
  socket.bindToPort(port, SERVER_NETADDR, "server");
  makeNonBlocking(socket.getFd());

  if (listen(socket.getFd(), SERVER_BACKLOG)) {
    throw std::runtime_error("Failed to listen");
  }

  sockaddr_in clientAddr = {};
  std::int64_t numFds;
  epoll_event events[MAX_EVENTS];
  std::vector<std::unique_ptr<Conn>> fd2Conn(MAX_EVENTS);

  std::int64_t epFd = epoll_create(1);
  epollCtlAdd(epFd, socket.getFd(), EPOLLIN | EPOLLOUT | EPOLLET);

  // the event loop
  while (true) {
    numFds = epoll_wait(epFd, events, MAX_EVENTS, -1);
    // connection fds
    for (std::int64_t i = 0; i < numFds; ++i) {
      if (events[i].data.fd == socket.getFd()) {
        socklen_t socketLen = sizeof(clientAddr);
        std::int64_t connFd =
            accept(socket.getFd(), reinterpret_cast<sockaddr*>(&clientAddr),
                   &socketLen);
        if (connFd < 0) {
          std::cerr << "accept() error";
          continue;
        }

        makeNonBlocking(connFd);
        epollCtlAdd(epFd, connFd, EPOLLIN | EPOLLET | EPOLLRDHUP | EPOLLHUP);

        if (static_cast<std::size_t>(connFd) >= fd2Conn.size()) {
          fd2Conn.resize(connFd + 1);
        }
        fd2Conn[connFd] = std::make_unique<Conn>(connFd, ConnState::REQ, 0);
      } else {
        auto& conn = fd2Conn[events[i].data.fd];
        if (!conn) {
          std::cerr << "Connection not found for fd: " << events[i].data.fd
                    << std::endl;
        }
        if (events[i].events & (EPOLLIN | EPOLLOUT)) {
          conn->io();
          if (conn->getState() == ConnState::END) {
            epoll_ctl(epFd, EPOLL_CTL_DEL, conn->getFd(), nullptr);
            fd2Conn[conn->getFd()].reset();
          }
        }
      }
      if (events[i].events & (EPOLLRDHUP | EPOLLHUP)) {
        epoll_ctl(epFd, EPOLL_CTL_DEL, events[i].data.fd, nullptr);
        fd2Conn[events[i].data.fd].reset();
      }
    }
  }
}

/**
 * @brief Get the Socket object
 *
 * @return Socket&
 */
Socket& Server::getSocket() { return socket; }