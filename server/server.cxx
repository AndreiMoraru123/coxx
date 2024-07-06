#include "server.hxx"

std::int32_t Server::oneRequest(std::int64_t connfd) {
  constexpr size_t k_max_msg = 4096;
  std::vector<char> rbuf(4 + k_max_msg + 1);
  errno = 0;

  std::string header(4, '\0');
  std::int32_t err = socket.readFull(connfd, header, 4);
  if (err) {
    if (errno == 0) {
      std::println("EOF");
    } else {
      std::cerr << "read() error" << std::endl;
    }
    return err;
  }

  // Copy the header back into rbuf
  std::memcpy(rbuf.data(), header.data(), 4);

  std::uint32_t len = 0;
  std::memcpy(&len, rbuf.data(), 4);
  if (len > k_max_msg) {
    std::println("too long");
    return -1;
  }

  // Read the request body
  std::string body(len, '\0');
  err = socket.readFull(connfd, body, len);
  if (err) {
    std::cerr << "read() error" << std::endl;
    return err;
  }

  // Copy the body back into rbuf
  std::memcpy(rbuf.data() + 4, body.data(), len);

  // Print the null temrinated string in the buffer
  rbuf[4 + len] = '\0';
  std::println("Client says {}", &rbuf[4]);

  // Reply using the same protocol
  const std::string reply = "world";
  std::vector<char> wbuf(4 + reply.size());
  len = static_cast<std::uint32_t>(reply.size());

  std::memcpy(wbuf.data(), &len, 4);
  std::memcpy(wbuf.data() + 4, reply.data(), len);

  std::string wbufStr(wbuf.begin(), wbuf.end());
  return socket.writeAll(connfd, wbufStr, len + 4);
}

void Server::doSomething(std::int64_t connfd) {
  std::vector<char> rbuf(64);
  ssize_t n = read(connfd, rbuf.data(), rbuf.size() - 1);
  if (n < 0) {
    std::cerr << "read() error" << std::endl;
    return;
  }

  std::println("Client says {}", std::string(rbuf.begin(), rbuf.begin() + n));

  std::string wbuf = "world";
  write(connfd, wbuf.c_str(), wbuf.length());
}

void Server::run() {
  socket.setOptions();
  socket.bindToPort(1234, 0, "server");

  if (listen(socket.getFd(), SOMAXCONN)) {
    throw std::runtime_error("Failed to listen");
  }

  while (true) {
    struct sockaddr_in client_addr = {};
    socklen_t socklen = sizeof(client_addr);
    std::int64_t connfd =
        accept(socket.getFd(), reinterpret_cast<struct sockaddr*>(&client_addr),
               &socklen);

    if (connfd == -1) {
      continue;  // error
    }

    while (true) {
      std::int32_t err = oneRequest(connfd);
      if (err) {
        break;
      }
    }

    close(connfd);
  }
}

Socket& Server::getSocket() { return socket; }