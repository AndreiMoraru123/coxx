#include "socket.hxx"

#include <sys/socket.h>
#include <unistd.h>

#include <stdexcept>

/**
 * @brief Construct a new Socket:: Socket object
 *
 *  Initializes a new socket for TCP/IP communication using IPv4 and stores the
 * socket handle.
 *
 *  The socket() syscall takes 3 integer arguments:
 *  - AF_INET is for IPv4. Use AF_INET6 for IPv6 or dual-stack sockets. This
 *    selects the IP level protocol. For simplicity, we’ll only consider IPv4.
 *  - SOCK_STREAM is for TCP. Use SOCK_DGRAM for UDP, which is not our concern.
 *  - The 3rd argument is 0 and typically specifies the protocol. For TCP
 *    sockets, this is usually left as 0 to select the default protocol.
 *
 *  @throws std::runtime_error if the socket fails to create.
 */
Socket::Socket() {
  _fd = socket(AF_INET, SOCK_STREAM, 0);
  if (_fd == -1) [[unlikely]] {
    throw std::runtime_error("Failed to create socket");
  }
}

/**
 * @brief Destroy the Socket:: Socket object
 *
 * Closes the socket file descriptor to free up the network resource.
 */
Socket::~Socket() { close(_fd); }

/**
 * @brief Sets socket options.
 *
 *  This method configures the socket to allow listening to a port.
 *  Most options are optional, except SO_REUSEADDR, which has to be enabled (set
 * to 1), otherwise bind() would fail when restarting the server.
 *
 * @throws std::runtime_error IF setting the socket option fails.
 */
void Socket::setOptions() const {
  constexpr std::int64_t val = 1;
  if (setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val)) == -1)
      [[unlikely]] {
    throw std::runtime_error("Failed to set socket options");
  }
}

/**
 * @brief Bind/Connect the Socket to a specified network address and port.
 *
 * This method further configures the socket to a specific port and network,
 * preparing it for either client or server use based on the connection type.
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
void Socket::configureConnection(std::int64_t port, std::uint32_t netaddr,
                                 const std::string &connectionType) const {
  sockaddr_in addr = {};
  addr.sin_family = AF_INET;
  addr.sin_port = ntohs(port);
  addr.sin_addr.s_addr = ntohl(netaddr);
  if (connectionType == "server") {
    if (bind(_fd, reinterpret_cast<sockaddr *>(&addr), sizeof(addr))) {
      throw std::runtime_error("Failed to bind to port");
    }
  } else if (connectionType == "client") {
    if (connect(_fd, reinterpret_cast<sockaddr *>(&addr), sizeof(addr))) {
      throw std::runtime_error("Failed to connect to port");
    }
  } else [[unlikely]] {
    throw std::invalid_argument("Invalid connection type");
  }
}

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
std::int32_t Socket::readFull(std::int64_t fd, std::string &buffer,
                              std::size_t numberOfBytes) const {
  buffer.resize(numberOfBytes);
  std::size_t bytesRead = 0;

  while (bytesRead < numberOfBytes) {
    ssize_t readBytes = read(fd, &buffer[bytesRead], numberOfBytes - bytesRead);
    if (readBytes <= 0) {
      return -1;  // Error, or unexpected EOF
    }
    bytesRead += static_cast<std::size_t>(readBytes);
  }
  buffer.resize(bytesRead);
  return 0;
}

/**
 * @brief Writes @p numberOfBytes bytes from the @p buffer to the file
 * descriptor @p fd.
 *
 * @param fd The file descriptor to which to write.
 * @param buffer A reference to a string containing the data to be written.
 * The function ensures that the string is resized to exactly fit the read
 * data.
 * @param numberOfBytes The number of bytes to write.
 * @return std::int32_t 0 on success, -1 if an error occurs during
 * writing
 */
std::int32_t Socket::writeAll(std::int64_t fd, std::string &buffer,
                              std::size_t numberOfBytes) const {
  buffer.resize(numberOfBytes);
  std::size_t bytesWrote = 0;

  while (bytesWrote < numberOfBytes) {
    ssize_t writtenBytes =
        write(fd, &buffer[bytesWrote], numberOfBytes - bytesWrote);
    if (writtenBytes <= 0) {
      return -1;  // Error
    }
    bytesWrote += static_cast<std::size_t>(writtenBytes);
  }
  buffer.resize(bytesWrote);
  return 0;
}

/**
 * @brief Get the Fd object
 *
 * @return the file descriptor of the Socket.
 */
int Socket::getFd() const { return _fd; }