﻿#ifndef _BYFXXM_ADDRESS_HPP_
#define _BYFXXM_ADDRESS_HPP_

#include "common.hpp"
#include <unordered_map>

namespace byfxxm {
class Address {
public:
  Address() = default;

  Address(std::initializer_list<std::pair<double, SharpValue>> list) {
    for (auto &elem : list) {
      Insert(elem.first, elem.second);
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

  void Insert(double key, const SharpValue &sharp) {
    _dict.insert(std::make_pair(key, sharp));
  }

private:
  std::unordered_map<double, SharpValue> _dict;
  std::vector<std::unique_ptr<double>> _buffer;
};
} // namespace byfxxm

#endif
