#include <gtest/gtest.h>

#include "map.h"
#include "map.hxx"

auto equalityCNode = [](CNode* lhs, CNode* rhs) {
  return lhs->code == rhs->code;
};

auto equalityNode = [](const std::unique_ptr<Node>& lhs, const Node& rhs) {
  return lhs->code == rhs.code;
};

class LookUpTest : public ::testing::Test {
 protected:
  void SetUp() override { initMap(&cMap); }

  CMap cMap;
  Map map;
};

TEST_F(LookUpTest, CMapInsertAndLookUpTest) {
  CNode node1 = {.code = 1};
  CNode node2 = {.code = 2};

  CMapInsert(&cMap, &node1);
  CMapInsert(&cMap, &node2);

  CNode* lookUp1 = CMapLookUp(&cMap, &node1, equalityCNode);
  CNode* lookUp2 = CMapLookUp(&cMap, &node2, equalityCNode);

  ASSERT_NE(lookUp1, nullptr);
  ASSERT_NE(lookUp2, nullptr);
  ASSERT_EQ(lookUp1->code, 1);
  ASSERT_EQ(lookUp2->code, 2);

  CNode dummyNode = {.code = 3};
  CNode* dummyLookUp = CMapLookUp(&cMap, &dummyNode, equalityCNode);

  ASSERT_EQ(dummyLookUp, nullptr);
}

TEST_F(LookUpTest, MapInsertAndLookUpTest) {
  auto node1 = std::make_unique<Node>();
  node1->code = 1;
  auto node2 = std::make_unique<Node>();
  node2->code = 2;

  mapInsert(map, std::move(node1));
  mapInsert(map, std::move(node2));

  Node node1ForLookUp = {.code = 1};
  Node node2ForLookUp = {.code = 2};

  std::unique_ptr<Node> lookUp1 = mapLookUp(map, node1ForLookUp, equalityNode);
  std::unique_ptr<Node> lookUp2 = mapLookUp(map, node2ForLookUp, equalityNode);

  ASSERT_NE(lookUp1, nullptr);
  ASSERT_NE(lookUp2, nullptr);
  ASSERT_EQ(lookUp1->code, 1);
  ASSERT_EQ(lookUp2->code, 2);

  Node dummyNode = {.code = 1};
  std::unique_ptr<Node> dummyLookUp = mapLookUp(map, dummyNode, equalityNode);

  ASSERT_EQ(dummyLookUp, nullptr);
}
