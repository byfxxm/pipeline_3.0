#ifndef _GPARSER_ADDRESS_HPP_
#define _GPARSER_ADDRESS_HPP_

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
      _dict.insert(
          std::make_pair(x.first, MakeUnique<_VType>(mempool, x.second)));
    }
  }

  [[nodiscard]] _Value &operator[](const _Key &key) {
    if (_dict.find(key) == _dict.end())
      _dict.insert(std::make_pair(key, MakeUnique<_VType>(mempool, nan)));

    return _dict.at(key);
  }

private:
  std::pmr::unordered_map<_Key, _Value> _dict{&mempool};
};
} // namespace byfxxm

#endif
