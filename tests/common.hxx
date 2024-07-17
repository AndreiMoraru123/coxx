#include "client/client.hxx"
#include "server/server.hxx"

constexpr std::int64_t SERVER_NETADDR = 0;
constexpr std::int64_t CLIENT_NETADDR = INADDR_LOOPBACK;
constexpr std::int8_t BUFFER_SIZE = 64;
constexpr std::int16_t backlog = SOMAXCONN;
constexpr std::int64_t MAX_ITERATIONS =
    1;  // has to always be 1 because of how I made the threading setup.

const std::string clientSays = "hello";
const std::string serverResponds = "world";