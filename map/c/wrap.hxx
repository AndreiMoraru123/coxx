#pragma once

#include "map.h"

using Node = CNode;
using Table = CTable;
using Map = CMap;

namespace map {

constexpr auto lookup = CMapLookUp;
constexpr auto insert = CMapInsert;
constexpr auto pop = CMapPop;
constexpr auto size = CMapSize;
constexpr auto destroy = CMapDestroy;

} // namespace map
