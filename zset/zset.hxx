#include <cmath>
#include <cstdint>
#include <cstring>
#include <functional>
#include <memory>
#include <string>

#include "avl/c/avl.h"
#include "map/c/wrap.hxx"

#define containerOf(ptr, type, member)                                         \
  ({                                                                           \
    const decltype(((type *)0)->member) *__mptr = (ptr);                       \
    (type *)((char *)__mptr - offsetof(type, member));                         \
  })

struct ZSet {
  AVLNode *tree = nullptr;
  Map map;
};

struct Key {
  Node node;
  std::string name;
  std::size_t len = 0;
};

struct ZNode {
  AVLNode tree;
  Node map;
  std::double_t score = 0;
  std::size_t len = 0;
  std::string name;
};

auto stringHash(const std::string &data) -> std::uint64_t;
auto zAdd(ZSet *set, const std::string &name, std::size_t len,
          std::double_t score) -> bool;
auto zLookUp(ZSet *set, const std::string &name, std::size_t len) -> ZNode *;
auto zPop(ZSet *set, const std::string &name, std::size_t len) -> ZNode *;
auto zQuery(ZSet *set, std::double_t score, const std::string &name,
            std::size_t len) -> ZNode *;
auto zOffset(ZNode *node, std::int64_t off) -> ZNode *;
void zDel(ZNode *node);
void zDispose(ZSet *set);