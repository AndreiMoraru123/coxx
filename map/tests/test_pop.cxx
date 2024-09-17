
#include <gtest/gtest.h>

#include "map.h"
#include "map.hxx"

auto equalityCNode = [](CNode* lhs, CNode* rhs) {
  return lhs->code == rhs->code;
};

auto equalityNode = [](const std::unique_ptr<Node>& lhs, const Node& rhs) {
  return lhs->code == rhs.code;
};

class PopTest : public ::testing::Test {
 protected:
  void SetUp() override { initMap(&cMap); }

  CMap cMap;
  Map map;
};

TEST_F(PopTest, CMapPopTest) {
  CNode node = {.code = 1};

  CMapInsert(&cMap, &node);

  CNode* lookUp = CMapLookUp(&cMap, &node, equalityCNode);

  ASSERT_NE(lookUp, nullptr);
  ASSERT_EQ(lookUp->code, 1);

  CMapPop(&cMap, &node, equalityCNode);
  lookUp = CMapLookUp(&cMap, &node, equalityCNode);

  ASSERT_EQ(lookUp, nullptr);
}

TEST_F(PopTest, MapPopTest) {
  auto node = std::make_unique<Node>();
  node->code = 1;

  mapInsert(map, std::move(node));

  Node nodeForLookUp = {.code = 1};
  std::unique_ptr<Node> lookUp = mapLookUp(map, nodeForLookUp, equalityNode);

  ASSERT_NE(lookUp, nullptr);
  ASSERT_EQ(lookUp->code, 1);

  (void)mapPop(map, nodeForLookUp, equalityNode);
  lookUp = mapLookUp(map, nodeForLookUp, equalityNode);

  ASSERT_EQ(lookUp, nullptr);
}
