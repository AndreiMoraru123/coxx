
#include "socket.hxx"

class Client {
 public:
  Client() = default;
  void run();

 public:
  Socket socket;
};
