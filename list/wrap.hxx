#include "list.h"

using List = DList;

namespace list {

inline constexpr auto init = listInit;
inline constexpr auto empty = listEmpty;
inline constexpr auto detach = listDetach;
inline constexpr auto insert = listInsert;

} // namespace list