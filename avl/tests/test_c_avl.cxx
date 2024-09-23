#include <cstdint>
#include <gtest/gtest.h>

#include <cassert>
#include <random>
#include <set>

#include "avl.h"

#define containerOf(ptr, type, member)                                         \
  ({                                                                           \
    const decltype(std::declval<type>().member) *__mptr = (ptr);               \
    (type *)((char *)__mptr - offsetof(type, member));                         \
  })

struct Data {
  AVLNode node;
  std::uint32_t val = 0;
};

struct Container {
  AVLNode *root = nullptr;
};

static void add(Container &c, std::uint32_t val) {
  Data *data = new Data();
  init(&data->node);
  data->val = val;

  AVLNode *curr = nullptr;  // current node
  AVLNode **from = &c.root; // the incoming pointer to the next node

  while (*from) { // tree search
    curr = *from;
    std::uint32_t nodeValue = containerOf(curr, Data, node)->val;
    from = (val < nodeValue) ? &curr->left : &curr->right;
  }

  *from = &data->node; // attach the new node
  data->node.parent = curr;
  c.root = fixAVL(&data->node);
}

static auto del(Container &c, std::uint32_t val) -> bool {
  AVLNode *curr = c.root;
  while (curr) {
    std::uint32_t nodeValue = containerOf(curr, Data, node)->val;
    if (val == nodeValue) {
      break;
    }
    curr = val < nodeValue ? curr->left : curr->right;
  }

  if (!curr) {
    return false;
  }

  c.root = delAVL(curr);
  delete containerOf(curr, Data, node);
  return true;
}

static void verify(AVLNode *parent, AVLNode *node) {
  if (!node)
    return;

  // verify subtrees recursively
  verify(node, node->left);
  verify(node, node->right);

  // the parent pointer is correct
  assert(node->parent == parent);

  // the auxiliary data is correct
  assert(node->count == 1 + count(node->left) + count(node->right));

  // the height invariant is correct
  std::uint32_t l = depth(node->left);
  std::uint32_t r = depth(node->right);
  assert(node->depth == 1 + std::max(l, r));
  assert(l == r || l + 1 == r || l == r + 1);

  // the data is ordered
  std::uint32_t val = containerOf(node, Data, node)->val;
  if (node->left) {
    assert(node->left->parent == node);
    assert(containerOf(node->left, Data, node)->val <= val);
  }
  if (node->right) {
    assert(node->right->parent == node);
    assert(containerOf(node->right, Data, node)->val >= val);
  }
}

static void extract(AVLNode *node, std::multiset<std::uint32_t> &extracted) {
  if (!node)
    return;

  extract(node->left, extracted);
  extracted.insert(containerOf(node, Data, node)->val);
  extract(node->right, extracted);
}

static void verify(Container &c, const std::multiset<std::uint32_t> &ref) {
  verify(nullptr, c.root);
  assert(count(c.root) == ref.size());
  std::multiset<std::uint32_t> extracted;
  extract(c.root, extracted);
  assert(extracted == ref);
}

static void dispose(Container &c) {
  while (c.root) {
    AVLNode *node = c.root;
    c.root = delAVL(c.root);
    delete containerOf(node, Data, node);
  }
}

static void testInsert(std::uint32_t size) {
  for (std::uint32_t val = 0; val < size; ++val) {
    Container c;
    std::multiset<std::uint32_t> ref;
    for (std::uint32_t i = 0; i < size; ++i) {
      if (i == val) {
        continue;
      }
      add(c, i);
      ref.insert(i);
    }

    verify(c, ref);

    add(c, val);
    ref.insert(val);
    verify(c, ref);
    dispose(c);
  }
}

static void testInsertDuplicate(std::uint32_t size) {
  for (std::uint32_t val = 0; val < size; ++val) {
    Container c;
    std::multiset<std::uint32_t> ref;
    for (std::uint32_t i = 0; i < size; ++i) {
      add(c, i);
      ref.insert(i);
    }

    verify(c, ref);

    add(c, val);
    ref.insert(val);
    verify(c, ref);
    dispose(c);
  }
}

static void testRemove(std::uint32_t size) {
  for (std::uint32_t val = 0; val < size; ++val) {
    Container c;
    std::multiset<std::uint32_t> ref;
    for (std::uint32_t i = 0; i < size; ++i) {
      add(c, i);
      ref.insert(i);
    }
    verify(c, ref);

    assert(del(c, val));
    ref.erase(val);
    verify(c, ref);
    dispose(c);
  }
}

static void testOffset(std::uint32_t size) {
  Container c;
  for (std::uint32_t i = 0; i < size; ++i) {
    add(c, i);
  }
  AVLNode *min = c.root;
  while (min->left) {
    min = min->left;
  }
  // for each starting rank
  for (std::uint32_t i = 0; i < size; ++i) {
    AVLNode *node = offsetAVL(min, static_cast<std::int64_t>(i));
    assert(containerOf(node, Data, node)->val == i);
    // test all possible offset
    for (std::uint32_t j = 0; j < size; ++j) {
      std::int64_t off =
          static_cast<std::int64_t>(j) - static_cast<std::int64_t>(i);
      const AVLNode *offNode = offsetAVL(node, off);
      assert(containerOf(offNode, Data, node)->val == j);
    }
    // out of range by one
    assert(!offsetAVL(node, -static_cast<std::int64_t>(i) - 1));
    assert(!offsetAVL(node, size - i));
  }

  dispose(c);
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

TEST_F(AVLTest, SequentialInsertionTest) {
  for (std::uint32_t i = 0; i < 1000; i += 3) {
    add(c, i);
    ref.insert(i);
    verify(c, ref);
  }
}

TEST_F(AVLTest, RandomInsertionTest) {
  std::mt19937 gen(rd());
  std::uniform_int_distribution<uint32_t> dis(0, 999);
  for (std::uint32_t i = 0; i < 100; i++) {
    std::uint32_t val = dis(gen);
    add(c, val);
    ref.insert(val);
    verify(c, ref);
  }
}

TEST_F(AVLTest, RandomDeletionTest) {
  std::mt19937 gen(rd());
  std::uniform_int_distribution<uint32_t> dis(0, 999);
  std::uint32_t val = dis(gen);

  auto it = ref.find(val);

  if (it == ref.end()) {
    ASSERT_FALSE(del(c, val));
  } else {
    ASSERT_TRUE(del(c, val));
    ref.erase(it);
  }

  verify(c, ref);
}

TEST_F(AVLTest, InsertionDeletionTest) {
  for (std::uint32_t i = 0; i < 200; ++i) {
    testInsert(i);
    testInsertDuplicate(i);
    testRemove(i);
  }
}

TEST_F(AVLTest, OffsetTest) { testOffset(200); }