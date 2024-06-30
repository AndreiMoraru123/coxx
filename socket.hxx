#include <netinet/ip.h>

#include <cstdint>
#include <string>

class Socket {
 public:
  Socket();
  ~Socket();
  void setOptions() const;
  void bindToPort(std::int64_t port, std::uint32_t netaddr,
                  std::string connectionType) const;
  std::int32_t readFull(std::int64_t fd, std::string &buffer, size_t n);
  std::int32_t writeAll(std::int64_t fd, std::string &buffer, size_t n);
  int getFd();

 private:
  int _fd;  // file descriptor
};