#pragma once
#include "memory.hpp"
#include "token.hpp"
#include <optional>
#include <string>
#include <unordered_map>

namespace byfxxm {
class Address {
public:
  using _Key = double;
  using _VType = double;
  using _Value = UniquePtr<_VType>;

  Address() = default;

  Address(std::initializer_list<std::pair<_Key, _VType>> list) {
    for (auto &x : list) {
      _dict.insert(std::make_pair(x.first, MakeUnique<_VType>(x.second)));
    }
  }

  _Value &operator[](const _Key &key) {
    if (_dict.find(key) == _dict.end())
      _dict.insert(std::make_pair(key, MakeUnique<_VType>(nan)));

    return _dict.at(key);
  }

  const _Value &operator[](const _Key &key) const { return _dict.at(key); }

  bool Contains(const _Key &key) const { return _dict.contains(key); }

  void Clear() { _dict.clear(); }

private:
  std::pmr::unordered_map<_Key, _Value> _dict;
};
} // namespace byfxxm
