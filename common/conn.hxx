#include <fcntl.h>
#include <unistd.h>

#include <cassert>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <print>
#include <vector>

constexpr std::size_t K_MAX_MSG = 4096;

enum class ConnState : std::uint8_t {
  REQ = 0,
  RES = 1,
  END = 2,
};

class Conn {
 public:
  Conn()
      : state(ConnState::REQ),
        _fd(-1),
        rbuf(),
        wbuf(),
        wbufSent(0),
        wbufSize(0),
        rbufSize(0) {
    rbuf.reserve(4 + K_MAX_MSG);
    wbuf.reserve(4 + K_MAX_MSG);
  }

  Conn(std::int64_t fd, ConnState state, std::size_t wbufSent)
      : state(state),
        _fd(fd),
        rbuf(),
        wbuf(),
        wbufSent(wbufSent),
        wbufSize(0),
        rbufSize(0) {
    rbuf.reserve(4 + K_MAX_MSG);
    wbuf.reserve(4 + K_MAX_MSG);
  }

  ~Conn();

  /**
   * @brief Get the connection file descriptor
   *
   * @return int Returns the file descriptor of the Conn.
   */
  int getFd();

  /**
   * @brief Get the connection state
   *
   * @return int Returns the state of the Conn.
   */
  ConnState getState();

  void io();

 private:
  ConnState state;
  std::int64_t _fd;
  std::vector<std::uint8_t> rbuf;
  std::vector<std::uint8_t> wbuf;
  std::size_t wbufSent;
  std::size_t wbufSize;
  std::size_t rbufSize;
  bool tryOneRequest();
  bool tryFlushBuffer();
  bool tryFillBuffer();
  void stateReq();
  void stateRes();
};
