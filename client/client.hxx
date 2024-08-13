#include <unistd.h>

#include <array>
#include <cstring>
#include <iostream>
#include <print>
#include <string>
#include <vector>

#include "common/conn.hxx"
#include "common/socket.hxx"

constexpr std::int64_t CLIENT_NETADDR = INADDR_LOOPBACK;

using CommandList = std::vector<std::string>;

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
   * @param commands The command to be send.
   * @return Error code indicating success (0) or failure (-1).
   */
  std::int32_t sendRequest(std::int64_t fd, const CommandList commands) const;

  /**
   * @brief Reads a response from the server.
   *
   * This function reads a response from the server by first reading a 4-byte
   * header that indicates the length of the message, and then reading the
   * message itself.
   *
   * @param fd The file descriptor from which the response is read.
   * @return Error code indicating success (0) or failure (-1).
   */
  std::int32_t readResponse(std::int64_t fd) const;

  /**
   * @brief Runs the client with the provided queries to the server.
   *
   * This function sets up the client socket, binds it to a port, and sends a
   * list of queries to the server. It then reads the responses from the server
   * for each query.
   *
   * @param commands The queries to send to the server.
   * @param port The port to run the client on.
   */
  void run(CommandList commands, std::int64_t port);

 private:
  Socket socket;
};
