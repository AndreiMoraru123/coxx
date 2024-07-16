#include "client.hxx"

std::int32_t Client::query(std::int64_t fd, std::string text) {
  constexpr std::size_t k_max_msg = 4096;
  std::uint32_t len = static_cast<std::uint32_t>(text.size());

  if (len > k_max_msg) {
    return -1;
  }

  std::vector<char> wbuf(4 + k_max_msg);
  std::memcpy(wbuf.data(), &len, 4);
  std::memcpy(wbuf.data() + 4, text.data(), len);

  std::string wbufStr(wbuf.begin(), wbuf.end());
  std::int32_t writeErr = socket.writeAll(fd, wbufStr, 4 + len);
  if (writeErr) {
    return writeErr;
  }

  // 4 bytes header
  std::string header(4, '\0');
  std::vector<char> rbuf(4 + k_max_msg + 1);
  std::int32_t readErr = socket.readFull(fd, header, 4);
  errno = 0;
  if (readErr) {
    if (errno == 0) {
      std::println("EOF");
    } else {
      std::cerr << "read() error" << std::endl;
    }
    return readErr;
  }

  // Copy the header back into rbuf
  std::memcpy(rbuf.data(), header.data(), 4);

  std::memcpy(&len, rbuf.data(), 4);
  if (len > k_max_msg) {
    std::println("too long");
    return -1;
  }

  // Read the reply body
  std::string body(len, '\0');
  readErr = socket.readFull(fd, body, len);
  if (readErr) {
    std::cerr << "read() error" << std::endl;
    return readErr;
  }

  // Copy the body back into rbuf
  std::memcpy(rbuf.data() + 4, body.data(), len);

  // Do something
  rbuf[4 + len] = '\0';
  std::println("Server says {}", &rbuf[4]);
  return 0;
}

void Client::run() {
  socket.setOptions();
  socket.bindToPort(1234, INADDR_LOOPBACK, "client");

  std::int32_t err = query(socket.getFd(), "hello1");
  if (err) {
    goto L_DONE;
  }

  err = query(socket.getFd(), "hello2");
  if (err) {
    goto L_DONE;
  }

  err = query(socket.getFd(), "hello3");
  if (err) {
    goto L_DONE;
  }

L_DONE:
  close(socket.getFd());
}

Socket& Client::getSocket() { return socket; }