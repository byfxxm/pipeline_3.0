#ifndef _BYFXXM_ADDRESS_HPP_
#define _BYFXXM_ADDRESS_HPP_

#include "common.hpp"
#include "memory.hpp"
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
      _dict.insert(std::make_pair(key, point.get()));
      _buffer.push_back(std::move(point));
    }

    return _dict.at(key);
  }

  void Insert(double key, double *sharp) {
    _dict.insert(std::make_pair(key, sharp));
  }

  void Insert(double key, GetSetSharp getset) {
    _dict.insert(std::make_pair(key, std::move(getset)));
  }

  void Insert(double key, SharpValue sharp) {
    _dict.insert(std::make_pair(key, std::move(sharp)));
  }

private:
  std::pmr::unordered_map<double, SharpValue> _dict{&mempool};
  std::pmr::vector<std::unique_ptr<double>> _buffer{&mempool};
};
} // namespace byfxxm

#endif
