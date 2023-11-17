#ifndef _BYFXXM_TYPEDEFS_HPP_
#define _BYFXXM_TYPEDEFS_HPP_

#include "token.hpp"
#include <concepts>
#include <limits>
#include <variant>

namespace byfxxm {
template <class T>
concept StreamConcept = requires(T t) {
  { t.get() } -> std::integral;
  { t.peek() } -> std::integral;
  { t.eof() } -> std::same_as<bool>;
};

template <class... Ts> struct Overloaded : Ts... {
  using Ts::operator()...;
};

struct Gtag {
  token::Kind code;
  double value;
};

inline constexpr bool operator==(const Gtag &lhs, const Gtag &rhs) {
  return lhs.code == rhs.code && lhs.value == rhs.value;
}

using GetSharp = std::function<double()>;
using SetSharp = std::function<void(double)>;
using GetSetSharp = std::tuple<GetSharp, SetSharp>;
using SharpValue = std::variant<double *, GetSetSharp>;

inline double Get(const SharpValue &key) {
  return std::visit(Overloaded{
                        [](double *p) { return *p; },
                        [](const GetSetSharp &getset) {
                          return std::get<GetSharp>(getset)();
                        },
                    },
                    key);
}

inline void Set(const SharpValue &key, double val) {
  std::visit(
      Overloaded{
          [&](double *p) { *p = val; },
          [&](const GetSetSharp &getset) { std::get<SetSharp>(getset)(val); },
      },
      key);
}

using Group = std::pmr::vector<double>;
using Value = std::variant<std::monostate, double, SharpValue, std::string,
                           bool, Gtag, Group>;
using GetRetVal = std::function<Value()>;

inline constexpr double nan = std::numeric_limits<double>::quiet_NaN();
inline constexpr bool IsNaN(double v) { return v != v; }
} // namespace byfxxm

#endif
