#pragma once

#include <cstdint>
#include <string>

enum class Serialize : std::uint8_t {
  NIL = 0,
  ERR = 1,
  STR = 2,
  INT = 4,
  ARR = 5,
};

namespace out {

void nil(std::string &out);
void str(std::string &out, const std::string &val);
void num(std::string &out, std::int64_t val);
void err(std::string &out, std::int32_t code, const std::string &msg);
void arr(std::string &out, std::uint32_t n);

}  // namespace out