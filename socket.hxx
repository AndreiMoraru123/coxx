#include <netinet/ip.h>

#include <cstdint>

class Socket {
 public:
  Socket();
  ~Socket();
  void setOptions();
  void bindToPort(std::int64_t);
  int getFd();

 private:
  int fd;  // file descriptor
};