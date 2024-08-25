#include <gtest/gtest.h>

#include "table.h"
#include "table.hxx"

class TableTest : public ::testing::Test {
 protected:
  void SetUp() override {
    initTable(&cTable);
    table = std::make_unique<Table>();
  }

  CTable cTable;
  std::unique_ptr<Table> table;
};

TEST_F(TableTest, DummyTest) {}