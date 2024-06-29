#include <netinet/ip.h>

#include <cstdint>
#include <string>

class Socket {
 public:
  Socket();
  ~Socket();
  void setOptions();
  void bindToPort(std::int64_t port, std::uint32_t netaddr,
                  std::string connectionType);
  int getFd();

 private:
  int fd;  // file descriptor
};