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
  std::int32_t operator()(std::uint8_t& requestData,
                          std::uint32_t requestLength, Response& responseCode,
                          std::uint8_t& responseValue,
                          std::uint32_t& responseLength);

 private:
  static std::map<std::string, std::string> commandMap;
  std::uint32_t parse(std::uint8_t& requestData, std::size_t length,
                      std::vector<std::string>& outputData);
  Response get(const std::vector<std::string>& commandList,
               std::uint8_t& responseValue, std::uint32_t& responseLength);
  Response set(const std::vector<std::string>& commandList,
               [[maybe_unused]] std::uint8_t& responseValue,
               [[maybe_unused]] std::uint32_t& responseLength);
  Response del(const std::vector<std::string>& commandList,
               [[maybe_unused]] std::uint8_t& responseValue,
               [[maybe_unused]] std::uint32_t& responseLength);
  bool isCommand(const std::string& word, const char* commandList);
};