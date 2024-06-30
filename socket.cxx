#include "socket.hxx"

#include <sys/socket.h>
#include <unistd.h>

#include <cassert>
#include <stdexcept>

Socket::Socket() {
  _fd = socket(AF_INET, SOCK_STREAM, 0);
  if (_fd == -1) {
    throw std::runtime_error("Failed to create socket");
  }
}

Socket::~Socket() { close(_fd); }

void Socket::setOptions() const {
  constexpr std::int64_t val = 1;
  if (setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val)) == -1) {
    throw std::runtime_error("Failed to set socket options");
  }
}

void Socket::bindToPort(std::int64_t port, std::uint32_t netaddr,
                        std::string connectionType) const {
  struct sockaddr_in addr = {};
  addr.sin_family = AF_INET;
  addr.sin_port = ntohs(port);
  addr.sin_addr.s_addr = ntohl(netaddr);
  if (connectionType == "server") {
    if (bind(_fd, reinterpret_cast<sockaddr *>(&addr), sizeof(addr))) {
      throw std::runtime_error("Failed to bind to port");
    }
  } else if (connectionType == "client") {
    if (connect(_fd, reinterpret_cast<sockaddr *>(&addr), sizeof(addr))) {
      throw std::runtime_error("Failed to bind to port");
    }
  } else {
    throw std::invalid_argument("Invalid connection type");
  }
}

std::int32_t Socket::readFull(std::int64_t fd, std::string &buffer, size_t n) {
  buffer.resize(n);
  size_t bytesRead = 0;

  while (bytesRead < n) {
    ssize_t rv = read(fd, &buffer[bytesRead], n - bytesRead);
    if (rv <= 0) {
      return -1;  // Error, or enexpected EOF
    }
    bytesRead += static_cast<size_t>(rv);
  }
  buffer.resize(bytesRead);
  return 0;
}

std::int32_t Socket::writeAll(std::int64_t fd, std::string &buffer, size_t n) {
  buffer.resize(n);
  size_t bytesWrote = 0;

  while (bytesWrote < n) {
    ssize_t rv = write(fd, &buffer[bytesWrote], n - bytesWrote);
    if (rv <= 0) {
      return -1;  // Error
    }
    bytesWrote += static_cast<size_t>(rv);
  }
  buffer.resize(bytesWrote);
  return 0;
}

int Socket::getFd() { return _fd; }