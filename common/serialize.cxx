#include "serialize.hxx"

namespace out {

void nil(std::string &out) {
  out.push_back(static_cast<std::underlying_type_t<Serialize>>(Serialize::NIL));
}

void str(std::string &out, const std::string &val) {
  out.push_back(static_cast<std::underlying_type_t<Serialize>>(Serialize::STR));
  auto len = static_cast<std::uint32_t>(val.size());
  out.append(reinterpret_cast<char *>(&len), 4);
  out.append(val);
}

void num(std::string &out, std::int64_t val) {
  out.push_back(static_cast<std::underlying_type_t<Serialize>>(Serialize::INT));
  out.append(reinterpret_cast<char *>(&val), 8);
}

void err(std::string &out, std::int32_t code, const std::string &msg) {
  out.push_back(static_cast<std::underlying_type_t<Serialize>>(Serialize::ERR));
  out.append(reinterpret_cast<char *>(&code), 4);
  auto len = static_cast<std::uint32_t>(msg.size());
  out.append(reinterpret_cast<char *>(&len), 4);
  out.append(msg);
}

void arr(std::string &out, std::uint32_t n) {
  out.push_back(static_cast<std::underlying_type_t<Serialize>>(Serialize::ARR));
  out.append(reinterpret_cast<char *>(&n), 4);
}

}  // namespace out
