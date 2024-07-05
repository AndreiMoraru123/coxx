
#include <cstring>

#include "socket.hxx"

class Server {
 public:
  Server() = default;
  void run();
  Socket& getSocket();

 private:
  Socket socket;
};

Socket& Server::getSocket() { return socket; }