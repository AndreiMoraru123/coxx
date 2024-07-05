
#include <cstring>

#include "socket.hxx"

class Server {
 public:
  Server() = default;
  void run();

 public:
  Socket socket;
};