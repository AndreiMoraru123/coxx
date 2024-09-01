#pragma once
#include <cassert>
#include <cstdint>
#include <cstring>
#include <map>
#include <print>
#include <string>
#include <vector>

#include "serialize.hxx"

constexpr std::int64_t PORT = 1234;
constexpr std::size_t MAX_MESSAGE_SIZE = 4096;
constexpr std::size_t MAX_NUM_ARGS = 1024;

#define containerOf(ptr, type, member)                  \
  ({                                                    \
    const decltype(((type*)0)->member)* __mptr = (ptr); \
    (type*)((char*)__mptr - offsetof(type, member));    \
  })

enum class Error : std::int32_t {
  UNKNOWN = 1,
  TOO_BIG = 2,
};

struct CommandMap {
  CMap db;
};

struct Entry {
  CNode node;
  std::string key;
  std::string val;
};

class Request {
 public:
  Request() = default;
  void operator()(std::vector<std::string>& commandList, std::string& out);

  std::uint32_t parse(std::uint8_t& requestData, std::size_t length,
                      std::vector<std::string>& outputData);

 private:
  static CommandMap commandMap;
  void keys([[maybe_unused]] std::vector<std::string>& cmd,
            std::string& output);
  void get(std::vector<std::string>& commandList, std::string& output);
  void set(std::vector<std::string>& commandList, std::string& output);
  void del(std::vector<std::string>& commandList, std::string& output);
  bool isCommand(const std::string& word, const char* commandList);
};