#ifndef _BYFXXM_ADDRESS_HPP_
#define _BYFXXM_ADDRESS_HPP_

#include "common.hpp"
#include <unordered_map>

namespace byfxxm {
class Address {
public:
  Address() = default;

  Address(std::initializer_list<std::pair<double, SharpValue>> list) {
    for (auto &it : list) {
      Insert(it.first, it.second);
    }
  }

  [[nodiscard]] SharpValue &operator[](double key) {
    if (_dict.find(key) == _dict.end()) {
      auto point = std::make_unique<double>(nan);
      Insert(key, SharpValue(point.get()));
      _buffer.push_back(std::move(point));
    }

    return _dict.at(key);
  }

  void Insert(double key, SharpValue sharp) {
    _dict.insert(std::make_pair(key, std::move(sharp)));
  }

private:
  std::unordered_map<double, SharpValue> _dict;
  std::vector<std::unique_ptr<double>> _buffer;
};
} // namespace byfxxm

#endif
