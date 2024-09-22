#include "serialize.hxx"

#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstring>
#include <utility>

namespace out {

void nil(std::string &out) {
  out.push_back(std::to_underlying(Serialize::NIL));
}

void str(std::string &out, const std::string &val) {
  out.push_back(std::to_underlying(Serialize::STR));
  auto len = static_cast<std::uint32_t>(val.size());
  out.append(reinterpret_cast<char *>(&len), 4);
  out.append(val);
}

void num(std::string &out, std::int64_t val) {
  out.push_back(std::to_underlying(Serialize::INT));
  out.append(reinterpret_cast<char *>(&val), 8);
}

void dbl(std::string &out, std::double_t val) {
  out.push_back(std::to_underlying(Serialize::DBL));
  out.append(reinterpret_cast<char *>(&val), 8);
}

void err(std::string &out, std::int32_t code, const std::string &msg) {
  out.push_back(std::to_underlying(Serialize::ERR));
  out.append(reinterpret_cast<char *>(&code), 4);
  auto len = static_cast<std::uint32_t>(msg.size());
  out.append(reinterpret_cast<char *>(&len), 4);
  out.append(msg);
}

void arr(std::string &out, std::uint32_t n) {
  out.push_back(std::to_underlying(Serialize::ARR));
  out.append(reinterpret_cast<char *>(&n), 4);
}

auto begin_arr(std::string &out) -> void * {
  out.push_back(std::to_underlying(Serialize::ARR));
  out.append("\0\0\0\0", 4);                       // filled in end_arr()
  return reinterpret_cast<void *>(out.size() - 4); // the `ctx` arg
}

void end_arr(std::string &out, void *ctx, std::uint32_t n) {
  auto pos = reinterpret_cast<std::size_t>(ctx);
  assert(out[pos - 1] == std::to_underlying(Serialize::ARR));
  std::memcpy(&out[pos], &n, 4);
}

} // namespace out
