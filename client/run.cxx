#include "client.hxx"

auto main() -> int {
  Client client;
  client.run(QUERY_LIST);
  return 0;
}