﻿#ifndef _BYFXXM_ADDRESS_HPP_
#define _BYFXXM_ADDRESS_HPP_

#include "common.hpp"
#include <unordered_map>

namespace byfxxm {
class Address {
public:
  [[nodiscard]] SharpValue &operator[](double key) {
    if (_dict.find(key) == _dict.end()) {
      auto point = std::make_unique<double>(nan);
      _dict.insert(std::make_pair(key, point.get()));
      _buffer.push_back(std::move(point));
    }

    return _dict.at(key);
  }

  void Insert(double key, double *addr) {
    _dict.insert(std::make_pair(key, addr));
  }

  void Insert(double key, GetSetSharp addr) {
    _dict.insert(std::make_pair(key, std::move(addr)));
  }

private:
  std::pmr::unordered_map<double, SharpValue> _dict{&mempool};
  std::pmr::vector<std::unique_ptr<double>> _buffer{&mempool};
};
} // namespace byfxxm

#endif
