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
        readBuffer(),
        writeBuffer(),
        writeBufferSent(0),
        writeBufferSize(0),
        readBufferSize(0) {
    readBuffer.reserve(4 + MAX_MESSAGE_SIZE);
    writeBuffer.reserve(4 + MAX_MESSAGE_SIZE);
  }

  /**
   * @brief Construct a new Connection object
   *
   * @param fd File descriptor for the connection
   * @param state Initial state of the connection
   * @param writeBufferSent Number of bytes sent in the write buffer
   */
  Connection(std::int64_t fd, ConnectionState state,
             std::size_t writeBufferSent)
      : _fd(fd),
        state(state),
        readBuffer(),
        writeBuffer(),
        writeBufferSent(writeBufferSent),
        writeBufferSize(0),
        readBufferSize(0) {
    readBuffer.reserve(4 + MAX_MESSAGE_SIZE);
    writeBuffer.reserve(4 + MAX_MESSAGE_SIZE);
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
   * @return the file descriptor of the Connection.
   */
  auto getFd() const -> int;

  /**
   * @brief Get the connection state
   *
   * @return the state of the Connection.
   */
  auto getState() const -> ConnectionState;

  void io();

 private:
  std::int64_t _fd;
  ConnectionState state;
  std::vector<std::uint8_t> readBuffer;
  std::vector<std::uint8_t> writeBuffer;
  std::size_t writeBufferSent;
  std::size_t writeBufferSize;
  std::size_t readBufferSize;
  Request request;
  auto tryOneRequest() -> bool;
  auto tryFlushBuffer() -> bool;
  auto tryFillBuffer() -> bool;
  void stateRequest();
  void stateResponse();
};
