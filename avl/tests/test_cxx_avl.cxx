
#include <gtest/gtest.h>

#include <cassert>
#include <random>
#include <set>

#include "avl.hxx"

#define containerOf(ptr, type, member)                           \
  ({                                                             \
    const decltype(std::declval<type>().member) *__mptr = (ptr); \
    (type *)((char *)__mptr - offsetof(type, member));           \
  })

struct Data {
  AVLNode node;
  std::uint32_t val = 0;
};

struct Container {
  AVLTree tree;
};

static void add(Container &c, std::uint32_t val) {
  Data *data = new Data();
  AVLTree::init(&data->node);
  data->val = val;

  // insert node into tree
  c.tree.insert(&data->node);

  if (c.tree.root()) {
    c.tree.fix(c.tree.root());
  }
}

static bool del(Container &c, std::uint32_t val) {
  auto it = c.tree.tree.begin();
  auto end = c.tree.tree.end();
  while (it != end) {
    Data *data = containerOf(&*it, Data, node);
    if (data->val == val) {
      c.tree.erase(&*it);
      delete data;
      return true;
    }
    ++it;
  }

  return false;
}

static void verify(AVLNode *parent, AVLNode *node, Container &c) {
  if (!node) return;

  AVLNode *leftNode = c.tree.find_left(node);
  AVLNode *rightNode = c.tree.find_right(node);

  // verify subtrees recursively
  verify(node, leftNode, c);
  verify(node, rightNode, c);

  // the parent pointer is correct
  ASSERT_EQ(node->parent, parent);

  // the auxiliary data is correct
  ASSERT_EQ(node->count,
            1 + AVLTree::count(leftNode) + AVLTree::count(rightNode));

  // the height invariant is correct
  std::uint32_t l = AVLTree::depth(c.tree.find_left(node));
  std::uint32_t r = AVLTree::depth(c.tree.find_right(node));
  ASSERT_EQ(node->depth, 1 + std::max(l, r));
  ASSERT_TRUE(l == r || l + 1 == r || l == r + 1);

  // the data is ordered
  std::uint32_t val = containerOf(node, Data, node)->val;
  if (c.tree.find_left(node)) {
    ASSERT_EQ(c.tree.find_left(node)->parent, node);
    ASSERT_LE(containerOf(c.tree.find_left(node), Data, node)->val, val);
  }
  if (c.tree.find_right(node)) {
    ASSERT_EQ(c.tree.find_right(node)->parent, node);
    ASSERT_GE(containerOf(c.tree.find_right(node), Data, node)->val, val);
  }
}

static void extract(AVLNode *node, std::multiset<std::uint32_t> &extracted,
                    Container &c) {
  if (!node) return;

  extract(c.tree.find_left(node), extracted, c);
  extracted.insert(containerOf(node, Data, node)->val);
  extract(c.tree.find_right(node), extracted, c);
}

static void verify(Container &c, const std::multiset<std::uint32_t> &ref) {
  verify(nullptr, c.tree.root(), c);
  ASSERT_EQ(AVLTree::count(c.tree.root()), ref.size());
  std::multiset<std::uint32_t> extracted;
  extract(c.tree.root(), extracted, c);
  ASSERT_EQ(extracted, ref);
}

static void dispose(Container &c) {
  while (c.tree.root()) {
    AVLNode *node = c.tree.root();
    c.tree.erase(node);
    delete containerOf(node, Data, node);
  }
}

class AVLTest : public ::testing::Test {
 public:
  Container c;
  std::multiset<std::uint32_t> ref;

  std::random_device rd;

  void TearDown() override { dispose(c); }
};

TEST_F(AVLTest, ContainerTest) {
  verify(c, {});
  add(c, 123);
  verify(c, {123});
  ASSERT_FALSE(del(c, 124));
  ASSERT_TRUE(del(c, 123));
  verify(c, {});
}