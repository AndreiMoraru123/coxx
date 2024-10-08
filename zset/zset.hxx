#include <cmath>
#include <cstdint>
#include <cstring>
#include <functional>
#include <memory>
#include <string>

#include "avl/c/wrap.hxx"
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

namespace zset {

auto add(ZSet *set, const std::string &name, std::size_t len,
         std::double_t score) -> bool;
auto lookup(ZSet *set, const std::string &name, std::size_t len) -> ZNode *;
auto pop(ZSet *set, const std::string &name, std::size_t len) -> ZNode *;
auto query(ZSet *set, std::double_t score, const std::string &name,
           std::size_t len) -> ZNode *;
auto offset(ZNode *node, std::int64_t off) -> ZNode *;
void del(ZNode *node);
void dispose(ZSet *set);

} // namespace zset