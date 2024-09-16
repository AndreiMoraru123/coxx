#include <gtest/gtest.h>

#include <algorithm>
#include <chrono>
#include <cstring>
#include <ranges>
#include <thread>

#include "client/client.hxx"
#include "server/server.hxx"

constexpr std::int64_t TEST_SERVER_NETADDR = 0;
constexpr std::int64_t TEST_CLIENT_NETADDR = INADDR_LOOPBACK;
constexpr std::int8_t TEST_BUFFER_SIZE = 64;
constexpr std::int16_t TEST_BACKLOG = SOMAXCONN;
constexpr std::int64_t TEST_MAX_ITERATIONS =
    1;  // has to always be 1 because of how I made the threading setup.

const std::string clientSays = "hello";
const std::string serverResponds = "world";