#pragma once
#include <cassert>
#include <cstdint>
#include <cstring>
#include <map>
#include <print>
#include <string>
#include <vector>

#include "entry.hxx"
#include "serialize.hxx"

constexpr std::int64_t PORT = 1234;
constexpr std::size_t MAX_MESSAGE_SIZE = 4096;
constexpr std::size_t MAX_NUM_ARGS = 1024;

enum class Error : std::int32_t {
  UNKNOWN = 1,
  TOO_BIG = 2,
  TYPE = 3,
  ARG = 4,
};

struct CommandMap {
  CMap db;
};

class Request {
public:
  Request() = default;
  void operator()(std::vector<std::string> &commandList, std::string &out);

  auto parse(std::uint8_t &requestData, std::size_t length,
             std::vector<std::string> &outputData) -> std::uint32_t;

private:
  static CommandMap commandMap;
  void keys([[maybe_unused]] std::vector<std::string> &commandList,
            std::string &output) const;
  void get(std::vector<std::string> &commandList, std::string &output) const;
  void set(std::vector<std::string> &commandList, std::string &output) const;
  void del(std::vector<std::string> &commandList, std::string &output) const;
  void zadd(std::vector<std::string> &commandList, std::string &output) const;
  void zrem(std::vector<std::string> &commandList, std::string &output) const;
  void zscore(std::vector<std::string> &commandList, std::string &output) const;
  void zquery(std::vector<std::string> &commandList, std::string &output) const;
  auto isCommand(const std::string &word, const char *commandList) const;
  auto expectZSet(std::string &output, std::string &s, Entry **entry) const;
};