#include "client.hxx"

auto main(int argc, char **argv) -> int {
  Client client;
  CommandList commands(argv + 1, argv + argc);
  client.run(commands, PORT);
  return 0;
}