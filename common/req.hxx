#pragma once
#include <cassert>
#include <cstdint>
#include <cstring>
#include <map>
#include <print>
#include <string>
#include <vector>

#include "map/c/map.h"

constexpr std::int64_t PORT = 1234;
constexpr std::size_t MAX_MESSAGE_SIZE = 4096;
constexpr std::size_t MAX_NUM_ARGS = 1024;

#define containerOf(ptr, type, member)                  \
  ({                                                    \
    const decltype(((type*)0)->member)* __mptr = (ptr); \
    (type*)((char*)__mptr - offsetof(type, member));    \
  })

/**
 * @enum Response
 * @brief Represents the state of a server response.
 *
 */
enum class Response : std::uint32_t {
  OK = 0,
  ERR = 1,
  NX = 2,
};

struct Entry {
  CNode node;
  std::string key;
  std::string val;
};

struct CommandMap {
  CMap db;
};

class Request {
 public:
  Request() = default;
  std::int32_t operator()(std::uint8_t& requestData,
                          std::uint32_t requestLength, Response& response,
                          std::uint8_t& responseValue,
                          std::uint32_t& responseLength);

 private:
  static CommandMap commandMap;
  std::uint32_t parse(std::uint8_t& requestData, std::size_t length,
                      std::vector<std::string>& outputData);
  Response get(std::vector<std::string>& commandList,
               std::uint8_t& responseValue, std::uint32_t& responseLength);
  Response set(std::vector<std::string>& commandList,
               [[maybe_unused]] std::uint8_t& responseValue,
               [[maybe_unused]] std::uint32_t& responseLength);
  Response del(std::vector<std::string>& commandList,
               [[maybe_unused]] std::uint8_t& responseValue,
               [[maybe_unused]] std::uint32_t& responseLength);
  bool isCommand(const std::string& word, const char* commandList);
};