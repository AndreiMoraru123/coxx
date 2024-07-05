
#include "socket.hxx"

class Client {
 public:
  Client() = default;
  void run();
  Socket& getSocket();

 private:
  Socket socket;
};

Socket& Client::getSocket() { return socket; }