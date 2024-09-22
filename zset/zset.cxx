#include "zset.hxx"
#include "avl/c/avl.h"
#include "map/c/map.h"
#include <cstddef>
#include <cstdint>

auto mapCmp = [](CNode *node, CNode *key) {
  ZNode *znode = containerOf(node, ZNode, map);
  Key *nodeKey = containerOf(key, Key, node);
  if (znode->len != nodeKey->len)
    return false;
  return 0 == std::memcmp(znode->name.data(), nodeKey->name.data(), znode->len);
};

auto stringHash(const std::string &data) -> std::uint64_t {
  std::uint32_t hash = 0x811C9DC5;
  for (const auto &letter : data) {
    hash = (hash + letter) * 0x01000193;
  }
  return hash;
}

static auto zLess(const AVLNode *lhs, std::double_t score, std::string name,
                  std::size_t len) -> bool {
  ZNode *zl = containerOf(lhs, ZNode, tree);
  if (zl->score != score) {
    return zl->score < score;
  }

  std::int64_t returnValue =
      std::memcmp(zl->name.data(), name.data(), std::min(zl->len, len));
  if (returnValue != 0) {
    return returnValue < 0;
  }

  return zl->len < len;
}

static auto zLess(const AVLNode *lhs, const AVLNode *rhs) -> bool {

  const ZNode *zr = containerOf(rhs, ZNode, tree);
  return zLess(lhs, zr->score, zr->name, zr->len);
}

static void treeAdd(ZSet *set, ZNode *node) {
  AVLNode *curr = nullptr;
  AVLNode **from = &set->tree;
  while (*from) {
    curr = *from;
    from = zLess(&node->tree, curr) ? &curr->left : &curr->right;
  }
  *from = &node->tree; // attach the new node
  node->tree.parent = curr;
  set->tree = fix(&node->tree);
}

auto zNew(const std::string &name, std::size_t len, std::double_t score)
    -> ZNode * {
  auto node = new ZNode();
  init(&node->tree);
  node->map.next = nullptr;
  node->map.code = stringHash(name);
  node->score = score;
  node->len = len;

  if (node->name.size() < len) {
    node->name.resize(len);
  }

  std::copy(name.begin(), name.begin() + len, node->name.begin());
  return node;
}

auto zLookUp(ZSet *set, const std::string &name, std::size_t len) -> ZNode * {
  if (!set->tree)
    return nullptr;

  Key key;
  key.node.code = stringHash(name);
  key.name = name;
  key.len = len;
  CNode const *found = CMapLookUp(&set->map, &key.node, mapCmp);
  return found ? containerOf(found, ZNode, map) : nullptr;
}

static void zUpdate(ZSet *set, ZNode *node, std::double_t score) {
  if (node->score == score) {
    return;
  }
  set->tree = del(&node->tree);
  node->score = score;
  init(&node->tree);
  treeAdd(set, node);
}

auto zAdd(ZSet *set, const std::string &name, std::size_t len,
          std::double_t score) -> bool {
  ZNode *node = zLookUp(set, name, len);
  if (node) { // update the score of an existing pair
    zUpdate(set, node, score);
    return false;
  } else {
    node = zNew(name, len, score);
    CMapInsert(&set->map, &node->map);
    treeAdd(set, node);
    return true;
  }
}

auto zPop(ZSet *set, const std::string &name, std::size_t len) -> ZNode * {
  if (!set->tree)
    return nullptr;

  Key key;
  key.node.code = stringHash(name);
  key.name = name;
  key.len = len;

  CNode *found = CMapPop(&set->map, &key.node, mapCmp);
  if (!found) {
    return nullptr;
  }

  ZNode *node = containerOf(found, ZNode, map);
  set->tree = del(&node->tree);
  return node;
}

auto zQuery(ZSet *set, std::double_t score, const std::string &name,
            std::size_t len) -> ZNode * {
  AVLNode *found = nullptr;
  AVLNode *curr = set->tree;
  while (curr) {
    if (zLess(curr, score, name, len)) {
      curr = curr->right;
    } else {
      found = curr; // candidate
      curr = curr->left;
    }
  }
  return found ? containerOf(found, ZNode, tree) : nullptr;
}

auto zOffset(ZNode *node, std::int64_t off) -> ZNode * {
  AVLNode *offsetNode = node ? offset(&node->tree, off) : nullptr;
  return offsetNode ? containerOf(offsetNode, ZNode, tree) : nullptr;
}

void zDel(ZNode *node) { free(node); }

void zDispose(ZSet *set) { CMapDestroy(&set->map); }