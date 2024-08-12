#include "server.hxx"

/**
 * @brief Registers events for a file descriptor with to an epoll instance.
 *
 * This function adds a file descriptor entry @p fd in the interest list of the
 * epoll file descriptor instance @p epollFd and registers it to monitor the
 * specified @p events. This allows the epoll instance to notify when the
 * specified events occur on the file descriptor.
 *
 * @param epollFd the epoll file descriptor
 * @param fd the target file descriptor to be monitored
 * @param events the events to be monitored
 */
static void registerEpollEvent(std::int64_t epollFd, std::int64_t fd,
                               std::uint32_t events) {
  epoll_event epollEvent;
  epollEvent.events = events;
  epollEvent.data.fd = fd;
  if (epoll_ctl(epollFd, EPOLL_CTL_ADD, fd, &epollEvent) == -1) {
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
 * polled with the POLLIN flag. For the connection fd (connectionFd) the state
 * of the connection object (Connection) determines the poll flag. In this
 * scenario, the poll flag is either reading (POLLIN) or writing (POLLOUT),
 * never both. After `poll` returns, the server gets notified by which file
 * descriptors are ready for reading/writing and can process the connections in
 * the pollArgs vector.
 *
 * @param port The port to run the server on.
 */
void Server::run(std::int64_t port) {
  socket.setOptions();
  socket.configureConnection(port, SERVER_NETADDR, "server");
  makeNonBlocking(socket.getFd());

  if (listen(socket.getFd(), SERVER_BACKLOG)) {
    throw std::runtime_error("Failed to listen");
  }

  sockaddr_in clientAddress = {};
  socklen_t socketAddressLength = sizeof(clientAddress);

  std::int64_t numFileDescriptors;
  std::array<epoll_event, MAX_EVENTS> events;
  std::vector<std::unique_ptr<Connection>> connectionByFileDescriptor(
      MAX_EVENTS);

  std::int64_t epollFd = epoll_create(1);
  registerEpollEvent(epollFd, socket.getFd(), EPOLLIN | EPOLLOUT | EPOLLET);

  // the event loop
  while (true) {
    numFileDescriptors = epoll_wait(epollFd, events.data(), MAX_EVENTS, -1);
    // connection fds
    for (std::int64_t i = 0; i < numFileDescriptors; ++i) {
      if (events[i].data.fd == socket.getFd()) {
        std::int64_t connectionFd =
            accept(socket.getFd(), reinterpret_cast<sockaddr*>(&clientAddress),
                   &socketAddressLength);
        if (connectionFd < 0) {
          std::cerr << "accept() error";
          continue;
        }

        makeNonBlocking(connectionFd);
        registerEpollEvent(epollFd, connectionFd,
                           EPOLLIN | EPOLLET | EPOLLRDHUP | EPOLLHUP);

        if (static_cast<std::size_t>(connectionFd) >=
            connectionByFileDescriptor.size()) {
          connectionByFileDescriptor.resize(connectionFd + 1);
        }
        connectionByFileDescriptor[connectionFd] =
            std::make_unique<Connection>(connectionFd, ConnectionState::REQ, 0);

      } else {
        auto& conn = connectionByFileDescriptor[events[i].data.fd];
        if (!conn) {
          std::cerr << "Connection not found for fd: " << events[i].data.fd
                    << std::endl;
        }
        if (events[i].events & (EPOLLIN | EPOLLOUT)) {
          conn->io();
          if (conn->getState() == ConnectionState::END) {
            epoll_ctl(epollFd, EPOLL_CTL_DEL, conn->getFd(), nullptr);
            connectionByFileDescriptor[conn->getFd()].reset();
          }
        }
      }
      if (events[i].events & (EPOLLRDHUP | EPOLLHUP)) {
        epoll_ctl(epollFd, EPOLL_CTL_DEL, events[i].data.fd, nullptr);
        close(events[i].data.fd);
        connectionByFileDescriptor[events[i].data.fd].reset();
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