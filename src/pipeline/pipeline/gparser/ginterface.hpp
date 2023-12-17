#ifndef _BYFXXM_GINTERFACE_HPP_
#define _BYFXXM_GINTERFACE_HPP_

#include "common.hpp"
#include <ranges>
#include <vector>

namespace byfxxm {
class Address;
class Ginterface {
public:
  using Params = std::pmr::vector<Gtag>;
  struct Utils {
    double value{Gtag::default_value};
    Params &params;
    Address *address{nullptr};
    const MarkSnapshot &mark_snapshot;
  };

  virtual ~Ginterface() = default;
  virtual void None(const Utils &) = 0;
  virtual void G0(const Utils &) = 0;
  virtual void G1(const Utils &) = 0;
  virtual void G2(const Utils &) = 0;
  virtual void G3(const Utils &) = 0;
  virtual void G4(const Utils &) = 0;
  virtual void N(const Utils &) = 0;
};
} // namespace byfxxm

#endif
