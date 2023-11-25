#ifndef _BYFXXM_GINTERFACE_HPP_
#define _BYFXXM_GINTERFACE_HPP_

#include "common.hpp"
#include <tuple>
#include <vector>

namespace byfxxm {
class Address;
class Ginterface {
public:
  using Gparams = std::pmr::vector<Gtag>;
  using Utils = std::tuple<const Gparams &, const Address *>;

  virtual ~Ginterface() = default;
  virtual bool None(const Utils &) = 0;
  virtual bool G0(const Utils &) = 0;
  virtual bool G1(const Utils &) = 0;
  virtual bool G2(const Utils &) = 0;
  virtual bool G3(const Utils &) = 0;
  virtual bool G4(const Utils &) = 0;
};
} // namespace byfxxm

#endif
