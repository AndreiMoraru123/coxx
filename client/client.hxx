#include <unistd.h>

#include <array>
#include <cstring>
#include <iostream>
#include <print>
#include <string>
#include <vector>

#include "common/socket.hxx"

constexpr std::int64_t PORT = 1234;
constexpr std::int64_t NETADDR = INADDR_LOOPBACK;

class Client {
 public:
  /**
   * @brief Construct a new Client object.
   *
   */
  Client() = default;
  /**
   * @brief Sends a request to the server.
   *
   * This function sends a request to the server by writing the length of the
   * message followed by the message itself to the specified file descriptor as
   the protocol demands:
   *
   * +-----+------+
   * | len | msg  |
   * +-----+------+
   *
   * @param fd The file descriptor to which the request is sent.
   * @param text The message to be send.
   * @return std::int32_t Error code indicating success (0) or failure (-1).
   */
  std::int32_t sendRequest(std::int64_t fd, std::string text) const;
  /**
   * @brief Reads a response from the server.
   *
   * This function reads a response from the server by first reading a 4-byte
   * header that indicates the length of the message, and then reading the
   * message itself.
   *
   * @param fd The file descriptor from which the response is read.
   * @return std::int32_t Error code indicating success (0) or failure (-1).
   */
  std::int32_t readResponse(std::int64_t fd) const;
  /**
   * @brief Runs the client.
   *
   * This function sets up the client socket, binds it to a port, and sends a
   * list of queries to the server. It then reads the responses from the server
   * for each query.
   */
  void run();
  /**
   * @brief Get the Socket object
   *
   * @return Socket&
   */
  Socket& getSocket();

 private:
  Socket socket;
  static constexpr std::size_t k_max_msg = 4096;
};
