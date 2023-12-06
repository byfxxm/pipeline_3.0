#ifndef _BYFXXM_ADDRESS_HPP_
#define _BYFXXM_ADDRESS_HPP_

#include "common.hpp"
#include <unordered_map>

namespace byfxxm {
class Address {
public:
  using _Key = double;
  using _Value = SharpValue;

  Address() = default;

  Address(std::initializer_list<std::pair<_Key, _Value>> list) {
    for (auto &elem : list) {
      Insert(elem.first, elem.second);
    }
  }

  [[nodiscard]] _Value &operator[](const _Key &key) {
    if (_dict.find(key) == _dict.end()) {
      auto point = std::make_unique<double>(nan);
      Insert(key, _Value(point.get()));
      _buffer.push_back(std::move(point));
    }

    return _dict.at(key);
  }

  void Insert(const _Key &key, const _Value &sharp) {
    _dict.insert(std::make_pair(key, sharp));
  }

private:
  std::unordered_map<_Key, SharpValue> _dict;
  std::vector<std::unique_ptr<double>> _buffer;
};
} // namespace byfxxm

#endif
