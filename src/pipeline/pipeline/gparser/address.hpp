#ifndef _BYFXXM_ADDRESS_HPP_
#define _BYFXXM_ADDRESS_HPP_

#include "common.hpp"
#include "memory.hpp"
#include <unordered_map>

namespace byfxxm {
class Address {
public:
  using _Key = double;
  using _VType = double;
  using _Value = std::variant<std::unique_ptr<_VType>, _VType *>;

  [[nodiscard]] _VType *operator[](const _Key &key) {
    if (_dict.find(key) == _dict.end())
      _dict.insert(std::make_pair(key, std::make_unique<_VType>(nan)));

    return std::visit(
        Overloaded{[](std::unique_ptr<_VType> &p) { return p.get(); },
                   [](_VType *p) { return p; }},
        _dict.at(key));
  }

  void Insert(const _Key &key, _VType *addr) {
    _dict.insert(std::make_pair(key, addr));
  }

  void Insert(const _Key &key, std::unique_ptr<_VType> addr) {
    _dict.insert(std::make_pair(key, std::move(addr)));
  }

private:
  std::pmr::unordered_map<_Key, _Value> _dict{&mempool};
};
} // namespace byfxxm

#endif
