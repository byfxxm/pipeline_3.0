#ifndef _BYFXXM_GINTERFACE_HPP_
#define _BYFXXM_GINTERFACE_HPP_

#include "common.hpp"
#include <vector>

#define byfxxm_VF(f) virtual bool f(const Utils &) = 0

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

  byfxxm_VF(None);
  byfxxm_VF(G0);
  byfxxm_VF(G1);
  byfxxm_VF(G2);
  byfxxm_VF(G3);
  byfxxm_VF(G4);
};
} // namespace byfxxm

#endif
