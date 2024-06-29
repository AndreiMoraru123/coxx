#include "socket.hxx"

#include <sys/socket.h>
#include <unistd.h>

#include <stdexcept>

Socket::Socket() {
  fd = socket(AF_INET, SOCK_STREAM, 0);
  if (fd == -1) {
    throw std::runtime_error("Failed to create socket");
  }
}

Socket::~Socket() { close(fd); }

void Socket::setOptions() {
  std::int64_t val = 1;
  if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val)) == -1) {
    throw std::runtime_error("Failed to set socket options");
  }
}

void Socket::bindToPort(std::int64_t port) {
  struct sockaddr_in addr = {};
  addr.sin_family = AF_INET;
  addr.sin_port = ntohs(port);
  addr.sin_addr.s_addr = ntohl(0);
  if (bind(fd, reinterpret_cast<sockaddr *>(&addr), sizeof(addr))) {
    throw std::runtime_error("Failed to bind to port");
  }
}

int Socket::getFd() { return fd; }