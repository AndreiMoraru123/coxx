#include "client.hxx"

std::int32_t Client::sendRequest(std::int64_t fd, std::string text) {
  std::uint32_t len = static_cast<std::uint32_t>(text.size());

  if (len > k_max_msg) {
    return -1;
  }

  std::vector<char> wbuf(4 + k_max_msg);
  std::memcpy(wbuf.data(), &len, 4);
  std::memcpy(wbuf.data() + 4, text.data(), len);

  std::string wbufStr(wbuf.begin(), wbuf.end());
  return socket.writeAll(fd, wbufStr, 4 + len);
}

std::int32_t Client::readResponse(std::int64_t fd) {
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

  std::uint32_t len = 0;
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

  std::array<std::string, 3> queryList = {"hello1", "hello2", "hello3"};
  for (auto& query : queryList) {
    std::int32_t sendErr = sendRequest(socket.getFd(), query);
    if (sendErr) {
      std::cerr << "send request error" << std::endl;
    }
  }

  for (std::size_t i = 0; i < queryList.size(); ++i) {
    std::int32_t readErr = readResponse(socket.getFd());
    if (readErr) {
      std::cerr << "read response error" << std::endl;
    }
  }
}

Socket& Client::getSocket() { return socket; }