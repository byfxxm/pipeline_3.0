#ifndef _BYFXXM_GINTERFACE_HPP_
#define _BYFXXM_GINTERFACE_HPP_

#include "common.hpp"
#include <vector>

#define byfxxm_Ginterface_PVF(f) virtual bool f(const Utils &) = 0

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

  byfxxm_Ginterface_PVF(None);
  byfxxm_Ginterface_PVF(G0);
  byfxxm_Ginterface_PVF(G1);
  byfxxm_Ginterface_PVF(G2);
  byfxxm_Ginterface_PVF(G3);
  byfxxm_Ginterface_PVF(G4);
};
} // namespace byfxxm

#endif
