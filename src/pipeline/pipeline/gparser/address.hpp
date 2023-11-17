#ifndef _BYFXXM_ADDRESS_HPP_
#define _BYFXXM_ADDRESS_HPP_

#include "common.hpp"
#include "memory.hpp"
#include <unordered_map>

namespace byfxxm {
class Address {
public:
  [[nodiscard]] SharpValue &operator[](double key) {
    if (_dict.find(key) == _dict.end()) {
      auto point = std::make_unique<double>(nan);
      _dict.insert(std::make_pair(
          key, SharpValue{[p = point.get()]() { return *p; },
                          [p = point.get()](double v) { *p = v; }}));
      _buffer.push_back(std::move(point));
    }

    return _dict.at(key);
  }

  void Insert(double key, double *sharp) {
    _dict.insert(
        std::make_pair(key, SharpValue{[=]() { return *sharp; },
                                       [=](double v) { *sharp = v; }}));
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
