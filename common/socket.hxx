#pragma once
#include <netinet/ip.h>

#include <cstdint>
#include <string>

class Socket {
 public:
  /**
   * @brief Construct a new Socket:: Socket object
   *
   *  @throws std::runtime_error if the socket fails to create.
   */
  Socket();

  /**
   * @brief Destroy the Socket:: Socket object
   *
   * Closes the socket file descriptor to free up the network resource.
   */
  ~Socket();

  /**
   * @brief Sets socket options.
   *
   * @throws std::runtime_error IF setting the socket option fails.
   */
  void setOptions() const;

  /**
   * @brief Bind/Connect the Socket to a specified network address and port.
   *
   * This method further configures the socket to a specific port and network,
   * preparing it for either client or server use based on the connection type.
   *
   * Currently supports only IPv4.
   *
   * @param port The port number to bind or connect the socket to. This port
   * must be available and not in use by another socket.
   * @param netaddr The network address to bind or connect the socket to. This
   * is typically the IP address in its numeric representation.
   * @param connectionType A string specifying the type of connection: "client"
   * for client sockets that initiate connections, or "server" for server
   * sockets that listen for connection. This determines how the binding is
   * handled internally.
   *
   * @throws std::invalid_argument If the 'connectionType' is neither "client"
   * nor "server".
   * @throws std::runtime_error If either type of connection fails due to issues
   * such as the port being in use, insufficient permissions, network errors or
   * whatnot.
   */
  void configureConnection(std::int64_t port, std::uint32_t netaddr,
                           const std::string &connectionType) const;

  /**
   * @brief Reads @p numberOfBytes bytes from the file descriptor @p fd into the
   * @p buffer.
   *
   * @param fd The file descriptor from which to read.
   * @param buffer A reference to a string where the read data will be stored.
   * The function ensures that the string is resized to exactly fit the read
   * data.
   * @param numberOfBytes The number of bytes to read.
   * @return 0 on success, -1 if an error occurs or EOF is
   * encountered before reading @p numberOfBytes bytes.
   */
  std::int32_t readFull(std::int64_t fd, std::string &buffer,
                        std::size_t numberOfBytes) const;

  /**
   * @brief Writes @p numberOfBytes bytes from the @p buffer to the file
   * descriptor @p fd.
   *
   * @param fd The file descriptor to which to write.
   * @param buffer A reference to a string containing the data to be written.
   * The function ensures that the string is resized to exactly fit the read
   * data.
   * @param numberOfBytes The number of bytes to write.
   * @return 0 on success, -1 if an error occurs during
   * writing
   */
  std::int32_t writeAll(std::int64_t fd, std::string &buffer,
                        std::size_t numberOfBytes) const;

  /**
   * @brief Get the Fd object
   *
   * @return the file descriptor of the Socket.
   */
  int getFd() const;

 private:
  /**
   * @brief The file descriptor associated with a Socket instance.
   *
   * This member stores the handle returned by the socket() syscall, or -1 if
   * the socket is not currently open. It's used internally for all socket
   * operations.
   */
  int _fd;
};