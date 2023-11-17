#ifndef _BYFXXM_GINTERFACE_HPP_
#define _BYFXXM_GINTERFACE_HPP_

#include "common.hpp"
#include <vector>

namespace byfxxm {
using Gparams = std::pmr::vector<Gtag>;
class Address;
class Ginterface {
public:
  virtual ~Ginterface() = default;
  virtual bool None(const Gparams &, const Address *) = 0;
  virtual bool G0(const Gparams &, const Address *) = 0;
  virtual bool G1(const Gparams &, const Address *) = 0;
  virtual bool G2(const Gparams &, const Address *) = 0;
  virtual bool G3(const Gparams &, const Address *) = 0;
  virtual bool G4(const Gparams &, const Address *) = 0;
};
} // namespace byfxxm

#endif
