#include "zset.hxx"

namespace zset {

constexpr auto add = zAdd;
constexpr auto lookup = zLookUp;
constexpr auto pop = zPop;
constexpr auto query = zQuery;
constexpr auto offset = zOffset;
constexpr auto del = zDel;
constexpr auto dispose = zDispose;

} // namespace zset