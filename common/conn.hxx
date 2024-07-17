#pragma once
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
  int _fd;
  ConnState state;
  std::vector<std::uint8_t> rbuf;
  std::vector<std::uint8_t> wbuf;
  std::size_t wbufSent;

  Conn()
      : _fd(-1),
        state(ConnState::REQ),
        rbuf(4 + K_MAX_MSG),
        wbuf(4 + K_MAX_MSG),
        wbufSent(0) {}

  Conn(std::int64_t fd, ConnState state, std::size_t wbufSent)
      : _fd(fd),
        state(state),
        rbuf(4 + K_MAX_MSG),
        wbuf(4 + K_MAX_MSG),
        wbufSent(wbufSent) {}

  ~Conn();
  void io();

 private:
  bool tryOneRequest();
  bool tryFlushBuffer();
  bool tryFillBuffer();
  void stateReq();
  void stateRes();
};
