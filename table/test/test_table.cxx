#include <gtest/gtest.h>

#include "table.h"
#include "table.hxx"

auto equalityCNode = [](CNode* lhs, CNode* rhs) {
  return lhs->code == rhs->code;
};

auto equalityNode = [](const std::unique_ptr<Node>& lhs,
                       const std::unique_ptr<Node>& rhs) {
  return lhs->code == rhs->code;
};

class TableTest : public ::testing::Test {
 protected:
  void SetUp() override { initMap(&cMap); }

  CMap cMap;
  Map map;
};

TEST_F(TableTest, CMapInsertAndLookUpTest) {
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

TEST_F(TableTest, MapInsertAndLookUpTest) {
  auto node1 = std::make_unique<Node>();
  node1->code = 1;
  auto node2 = std::make_unique<Node>();
  node2->code = 2;

  mapInsert(map, std::move(node1));
  mapInsert(map, std::move(node2));

  auto node1ForLookUp = std::make_unique<Node>();
  node1ForLookUp->code = 1;
  auto node2ForLookUp = std::make_unique<Node>();
  node2ForLookUp->code = 2;

  std::unique_ptr<Node>* lookUp1 = mapLookUp(map, node1ForLookUp, equalityNode);
  std::unique_ptr<Node>* lookUp2 = mapLookUp(map, node2ForLookUp, equalityNode);

  ASSERT_NE(lookUp1, nullptr);
  ASSERT_NE(lookUp2, nullptr);
  ASSERT_EQ((*lookUp1)->code, 1);
  ASSERT_EQ((*lookUp2)->code, 2);

  auto dummyNode = std::make_unique<Node>();
  dummyNode->code = 3;
  std::unique_ptr<Node>* dummyLookUp = mapLookUp(map, dummyNode, equalityNode);

  ASSERT_EQ(dummyLookUp, nullptr);
}