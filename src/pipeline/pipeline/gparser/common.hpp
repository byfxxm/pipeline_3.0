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

using GetSharpValue = std::function<double()>;
using SetSharpValue = std::function<void(double)>;
using GetSetSharp = std::tuple<GetSharpValue, SetSharpValue>;
using SharpValue = std::variant<double *, GetSetSharp>;

using Group = std::pmr::vector<double>;
using Value = std::variant<std::monostate, double, SharpValue, std::string,
                           bool, Gtag, Group>;
using GetRetVal = std::function<Value()>;

inline constexpr double nan = std::numeric_limits<double>::quiet_NaN();
inline constexpr bool IsNaN(double v) { return v != v; }

inline double Get(const SharpValue &key) {
  return std::visit(Overloaded{
                        [](double *p) { return *p; },
                        [](const GetSetSharp &getset) {
                          return std::get<GetSharpValue>(getset)();
                        },
                    },
                    key);
}

inline void Set(const SharpValue &key, double val) {
  return std::visit(Overloaded{
                        [&](double *p) { *p = val; },
                        [&](const GetSetSharp &getset) {
                          std::get<SetSharpValue>(getset)(val);
                        },
                    },
                    key);
}
} // namespace byfxxm

#endif
