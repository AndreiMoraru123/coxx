#pragma once
#include <fcntl.h>
#include <unistd.h>

#include <cassert>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <map>
#include <memory>
#include <print>
#include <vector>

#include "req.hxx"

/**
 * @enum ConnectionState
 * @brief Represents the state of a connection.
 *
 */
enum class ConnectionState : std::uint8_t {
  REQ = 0, /** Request state */
  RES = 1, /** Response state */
  END = 2, /** End State */
};

/**
 * @class Connection
 * @brief  Manages a connection's state and buffers.
 *
 */
class Connection {
 public:
  /**
   * @brief Construct a new Connection object
   *
   */
  Connection()
      : _fd(-1),
        state(ConnectionState::REQ),
        rbuf(),
        wbuf(),
        wbufSent(0),
        wbufSize(0),
        rbufSize(0) {
    rbuf.reserve(4 + K_MAX_MSG);
    wbuf.reserve(4 + K_MAX_MSG);
  }

  /**
   * @brief Construct a new Connection object
   *
   * @param fd File descriptor for the connection
   * @param state Initial state of the connection
   * @param wbufSent Number of bytes sent in the write buffer
   */
  Connection(std::int64_t fd, ConnectionState state, std::size_t wbufSent)
      : _fd(fd),
        state(state),
        rbuf(),
        wbuf(),
        wbufSent(wbufSent),
        wbufSize(0),
        rbufSize(0) {
    rbuf.reserve(4 + K_MAX_MSG);
    wbuf.reserve(4 + K_MAX_MSG);
  }

  /**
   * @brief Destroy the Connection object
   *
   * Closes the connection file descriptor to free up the network resource.
   */
  ~Connection();

  /**
   * @brief Get the connection file descriptor
   *
   * @return int Returns the file descriptor of the Connection.
   */
  int getFd() const;

  /**
   * @brief Get the connection state
   *
   * @return int Returns the state of the Connection.
   */
  ConnectionState getState() const;

  void io();

 private:
  std::int64_t _fd;
  ConnectionState state;
  std::vector<std::uint8_t> rbuf;
  std::vector<std::uint8_t> wbuf;
  std::size_t wbufSent;
  std::size_t wbufSize;
  std::size_t rbufSize;
  Request request;
  bool tryOneRequest();
  bool tryFlushBuffer();
  bool tryFillBuffer();
  void stateRequest();
  void stateResponse();
};
