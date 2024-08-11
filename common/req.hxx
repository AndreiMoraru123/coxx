#pragma once
#include <cassert>
#include <cstdint>
#include <cstring>
#include <map>
#include <print>
#include <string>
#include <vector>

constexpr std::int64_t PORT = 1234;
constexpr std::size_t K_MAX_MSG = 4096;
constexpr std::size_t K_MAX_ARGS = 1024;

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

class Request {
 public:
  Request() = default;
  std::int32_t operator()(std::uint8_t& request, std::uint32_t reqLen,
                          Response& resCode, std::uint8_t& response,
                          std::uint32_t& resLen);

 private:
  static std::map<std::string, std::string> cmdMap;
  std::uint32_t parse(std::uint8_t& data, std::size_t len,
                      std::vector<std::string>& out);
  Response get(const std::vector<std::string>& cmd, std::uint8_t& response,
               std::uint32_t& responseLength);
  Response set(const std::vector<std::string>& cmd,
               [[maybe_unused]] std::uint8_t& response,
               [[maybe_unused]] std::uint32_t& responseLength);
  Response del(const std::vector<std::string>& cmd,
               [[maybe_unused]] std::uint8_t& response,
               [[maybe_unused]] std::uint32_t& responseLength);
  bool isCmd(const std::string& word, const char* cmd);
};