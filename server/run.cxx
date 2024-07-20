#include "server.hxx"

auto main() -> int {
  Server server;
  server.run(PORT);
  return 0;
}