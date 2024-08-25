#include <gtest/gtest.h>

#include "map.h"
#include "map.hxx"

class DestroyTest : public ::testing::Test {
 protected:
  void SetUp() override { initMap(&cMap); }

  CMap cMap;
  Map map;
};

TEST_F(DestroyTest, CMapDestroyTest) {
  CNode node1 = {.code = 1};
  CNode node2 = {.code = 2};

  CMapInsert(&cMap, &node1);
  CMapInsert(&cMap, &node2);

  ASSERT_EQ(CMapSize(&cMap), 2);

  CMapDestroy(&cMap);

  ASSERT_EQ(CMapSize(&cMap), 0);
}

TEST_F(DestroyTest, MapDestroyTest) {
  auto node1 = std::make_unique<Node>();
  node1->code = 1;
  auto node2 = std::make_unique<Node>();
  node2->code = 1;

  mapInsert(map, std::move(node1));
  mapInsert(map, std::move(node2));

  ASSERT_EQ(mapSize(map), 2);

  mapDestroy(map);

  ASSERT_EQ(mapSize(map), 0);
}
