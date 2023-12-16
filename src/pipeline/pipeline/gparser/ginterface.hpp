#ifndef _BYFXXM_GINTERFACE_HPP_
#define _BYFXXM_GINTERFACE_HPP_

#include "common.hpp"
#include <vector>

namespace byfxxm {
class Address;
class Ginterface {
public:
  using Gparams = std::pmr::vector<Gtag>;
  struct Utils {
    const Gparams &params;
    const Address *address{nullptr};
    const MarkSnapshot &mark_snapshot;
  };

  virtual ~Ginterface() = default;
  virtual void None(const Utils &) = 0;
  virtual void G0(const Utils &) = 0;
  virtual void G1(const Utils &) = 0;
  virtual void G2(const Utils &) = 0;
  virtual void G3(const Utils &) = 0;
  virtual void G4(const Utils &) = 0;
};
} // namespace byfxxm

#endif
