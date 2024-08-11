#include "server.hxx"

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
static void makeNonBlocking(std::int64_t fd) {
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
 * @brief Runs the server event loop.
 *
 * This function first sets up the arguments for polling. The listening fd is
 * polled with the POLLIN flag. For the connection fd (connFd) the state of the
 * connection object (Connection) determines the poll flag. In this scenario,
 * the poll flag is either reading (POLLIN) or writing (POLLOUT), never both.
 * After `poll` returns, the server gets notified by which file descriptors are
 * ready for reading/writing and can process the connections in the pollArgs
 * vector.
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
  socklen_t socketLen = sizeof(clientAddr);

  std::int64_t numFds;
  std::array<epoll_event, MAX_EVENTS> events;
  std::vector<std::unique_ptr<Connection>> fd2Conn(MAX_EVENTS);

  std::int64_t epFd = epoll_create(1);
  epollCtlAdd(epFd, socket.getFd(), EPOLLIN | EPOLLOUT | EPOLLET);

  // the event loop
  while (true) {
    numFds = epoll_wait(epFd, events.data(), MAX_EVENTS, -1);
    // connection fds
    for (std::int64_t i = 0; i < numFds; ++i) {
      if (events[i].data.fd == socket.getFd()) {
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
        fd2Conn[connFd] =
            std::make_unique<Connection>(connFd, ConnectionState::REQ, 0);

      } else {
        auto& conn = fd2Conn[events[i].data.fd];
        if (!conn) {
          std::cerr << "Connection not found for fd: " << events[i].data.fd
                    << std::endl;
        }
        if (events[i].events & (EPOLLIN | EPOLLOUT)) {
          conn->io();
          if (conn->getState() == ConnectionState::END) {
            epoll_ctl(epFd, EPOLL_CTL_DEL, conn->getFd(), nullptr);
            fd2Conn[conn->getFd()].reset();
          }
        }
      }
      if (events[i].events & (EPOLLRDHUP | EPOLLHUP)) {
        epoll_ctl(epFd, EPOLL_CTL_DEL, events[i].data.fd, nullptr);
        close(events[i].data.fd);
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