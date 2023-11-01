#ifndef _BYFXXM_PREDICATE_HPP_
#define _BYFXXM_PREDICATE_HPP_

#include "address.hpp"
#include "exception.hpp"
#include "ginterface.hpp"
#include "token.hpp"
#include <algorithm>
#include <string>
#include <type_traits>
#include <variant>

#define byfxxm_IsType(v, type)                                                 \
  (std::is_same_v<std::remove_cvref_t<decltype(v)>, type>)
#define byfxxm_IsDouble(v) byfxxm_IsType(v, double)
#define byfxxm_IsDoublePtr(v) byfxxm_IsType(v, double *)
#define byfxxm_IsString(v) byfxxm_IsType(v, std::string)
#define byfxxm_IsGroup(v) byfxxm_IsType(v, Group)
#define byfxxm_IsBool(v) byfxxm_IsType(v, bool)

namespace byfxxm {
namespace predicate {
inline constexpr auto Plus = [](const Value &lhs, const Value &rhs) {
  return std::visit(
      [](auto &&l, auto &&r) -> Value {
        if constexpr (byfxxm_IsDoublePtr(l) && byfxxm_IsDoublePtr(r))
          return Value{*l + *r};
        else if constexpr (byfxxm_IsDoublePtr(l) && byfxxm_IsDouble(r))
          return Value{*l + r};
        else if constexpr (byfxxm_IsDouble(l) && byfxxm_IsDoublePtr(r))
          return Value{l + *r};
        else if constexpr (byfxxm_IsDouble(l) && byfxxm_IsDouble(r))
          return Value{l + r};
        else if constexpr (byfxxm_IsString(l) && byfxxm_IsString(r))
          return Value{l + r};
        else
          throw AbstreeException("plus error");
      },
      lhs, rhs);
};

inline constexpr auto Minus = [](const Value &lhs, const Value &rhs) {
  return std::visit(
      [](auto &&l, auto &&r) -> Value {
        if constexpr (byfxxm_IsDoublePtr(l) && byfxxm_IsDoublePtr(r))
          return Value{*l - *r};
        else if constexpr (byfxxm_IsDoublePtr(l) && byfxxm_IsDouble(r))
          return Value{*l - r};
        else if constexpr (byfxxm_IsDouble(l) && byfxxm_IsDoublePtr(r))
          return Value{l - *r};
        else if constexpr (byfxxm_IsDouble(l) && byfxxm_IsDouble(r))
          return Value{l - r};
        else
          throw AbstreeException("minus error");
      },
      lhs, rhs);
};

inline constexpr auto Multi = [](const Value &lhs, const Value &rhs) {
  return std::visit(
      [](auto &&l, auto &&r) -> Value {
        if constexpr (byfxxm_IsDoublePtr(l) && byfxxm_IsDoublePtr(r))
          return Value{*l * *r};
        else if constexpr (byfxxm_IsDoublePtr(l) && byfxxm_IsDouble(r))
          return Value{*l * r};
        else if constexpr (byfxxm_IsDouble(l) && byfxxm_IsDoublePtr(r))
          return Value{l * *r};
        else if constexpr (byfxxm_IsDouble(l) && byfxxm_IsDouble(r))
          return Value{l * r};
        else
          throw AbstreeException("multiple error");
      },
      lhs, rhs);
};

inline constexpr auto Div = [](const Value &lhs, const Value &rhs) {
  return std::visit(
      [](auto &&l, auto &&r) -> Value {
        if constexpr (byfxxm_IsDoublePtr(l) && byfxxm_IsDoublePtr(r))
          return Value{*l / *r};
        else if constexpr (byfxxm_IsDoublePtr(l) && byfxxm_IsDouble(r))
          return Value{*l / r};
        else if constexpr (byfxxm_IsDouble(l) && byfxxm_IsDoublePtr(r))
          return Value{l / *r};
        else if constexpr (byfxxm_IsDouble(l) && byfxxm_IsDouble(r))
          return Value{l / r};
        else
          throw AbstreeException("divide error");
      },
      lhs, rhs);
};

inline constexpr auto Assign = [](Value &lhs, const Value &rhs) {
  return std::visit(
      [](auto &&l, auto &&r) -> Value {
        if constexpr (byfxxm_IsDoublePtr(l) && byfxxm_IsDoublePtr(r))
          *l = *r;
        else if constexpr (byfxxm_IsDoublePtr(l) && byfxxm_IsDouble(r))
          *l = r;
        else
          throw AbstreeException("assign error");

        return Value{l};
      },
      lhs, rhs);
};

inline constexpr auto Neg = [](const Value &value) {
  return std::visit(
      [](auto &&v) -> Value {
        if constexpr (byfxxm_IsDouble(v))
          return Value{-v};
        else if constexpr (byfxxm_IsDoublePtr(v))
          return Value{-*v};
        else
          throw AbstreeException("negative error");
      },
      value);
};

inline constexpr auto Pos = [](const Value &value) {
  return std::visit(
      [](auto &&v) -> Value {
        if constexpr (byfxxm_IsDouble(v))
          return Value{v};
        else if constexpr (byfxxm_IsDoublePtr(v))
          return Value{*v};
        else
          throw AbstreeException("positive error");
      },
      value);
};

inline constexpr auto Sharp = [](const Value &value, Address *addr) {
  if (!addr)
    throw AbstreeException();

  return std::visit(
      [&](auto &&v) -> Value {
        if constexpr (byfxxm_IsDouble(v))
          return (*addr)[v].Get();
        else if constexpr (byfxxm_IsDoublePtr(v))
          return (*addr)[*v].Get();
        else
          throw AbstreeException("sharp error");
      },
      value);
};

inline constexpr auto GT = [](const Value &lhs, const Value &rhs) {
  return std::visit(
      [](auto &&l, auto &&r) -> Value {
        if constexpr (byfxxm_IsDoublePtr(l) && byfxxm_IsDoublePtr(r))
          return Value{*l > *r};
        else if constexpr (byfxxm_IsDoublePtr(l) && byfxxm_IsDouble(r))
          return Value{*l > r};
        else if constexpr (byfxxm_IsDouble(l) && byfxxm_IsDoublePtr(r))
          return Value{l > *r};
        else if constexpr (byfxxm_IsDouble(l) && byfxxm_IsDouble(r))
          return Value{l > r};
        else
          throw AbstreeException();
      },
      lhs, rhs);
};

inline constexpr auto GE = [](const Value &lhs, const Value &rhs) {
  return std::visit(
      [](auto &&l, auto &&r) -> Value {
        if constexpr (byfxxm_IsDoublePtr(l) && byfxxm_IsDoublePtr(r))
          return Value{*l >= *r};
        else if constexpr (byfxxm_IsDoublePtr(l) && byfxxm_IsDouble(r))
          return Value{*l >= r};
        else if constexpr (byfxxm_IsDouble(l) && byfxxm_IsDoublePtr(r))
          return Value{l >= *r};
        else if constexpr (byfxxm_IsDouble(l) && byfxxm_IsDouble(r))
          return Value{l >= r};
        else
          throw AbstreeException();
      },
      lhs, rhs);
};

inline constexpr auto LT = [](const Value &lhs, const Value &rhs) {
  return std::visit(
      [](auto &&l, auto &&r) -> Value {
        if constexpr (byfxxm_IsDoublePtr(l) && byfxxm_IsDoublePtr(r))
          return Value{*l < *r};
        else if constexpr (byfxxm_IsDoublePtr(l) && byfxxm_IsDouble(r))
          return Value{*l < r};
        else if constexpr (byfxxm_IsDouble(l) && byfxxm_IsDoublePtr(r))
          return Value{l < *r};
        else if constexpr (byfxxm_IsDouble(l) && byfxxm_IsDouble(r))
          return Value{l < r};
        else
          throw AbstreeException();
      },
      lhs, rhs);
};

inline constexpr auto LE = [](const Value &lhs, const Value &rhs) {
  return std::visit(
      [](auto &&l, auto &&r) -> Value {
        if constexpr (byfxxm_IsDoublePtr(l) && byfxxm_IsDoublePtr(r))
          return Value{*l <= *r};
        else if constexpr (byfxxm_IsDoublePtr(l) && byfxxm_IsDouble(r))
          return Value{*l <= r};
        else if constexpr (byfxxm_IsDouble(l) && byfxxm_IsDoublePtr(r))
          return Value{l <= *r};
        else if constexpr (byfxxm_IsDouble(l) && byfxxm_IsDouble(r))
          return Value{l <= r};
        else
          throw AbstreeException();
      },
      lhs, rhs);
};

inline constexpr auto EQ = [](const Value &lhs, const Value &rhs) {
  return std::visit(
      [](auto &&l, auto &&r) -> Value {
        if constexpr (byfxxm_IsDoublePtr(l) && byfxxm_IsDoublePtr(r))
          return Value{*l == *r};
        else if constexpr (byfxxm_IsDoublePtr(l) && byfxxm_IsDouble(r))
          return Value{*l == r};
        else if constexpr (byfxxm_IsDouble(l) && byfxxm_IsDoublePtr(r))
          return Value{l == *r};
        else if constexpr (byfxxm_IsDouble(l) && byfxxm_IsDouble(r))
          return Value{l == r};
        else
          throw AbstreeException();
      },
      lhs, rhs);
};

inline constexpr auto NE = [](const Value &lhs, const Value &rhs) {
  return std::visit(
      [](auto &&l, auto &&r) -> Value {
        if constexpr (byfxxm_IsDoublePtr(l) && byfxxm_IsDoublePtr(r))
          return Value{*l != *r};
        else if constexpr (byfxxm_IsDoublePtr(l) && byfxxm_IsDouble(r))
          return Value{*l != r};
        else if constexpr (byfxxm_IsDouble(l) && byfxxm_IsDoublePtr(r))
          return Value{l != *r};
        else if constexpr (byfxxm_IsDouble(l) && byfxxm_IsDouble(r))
          return Value{l != r};
        else
          throw AbstreeException();
      },
      lhs, rhs);
};

template <token::Kind K>
inline constexpr auto Gcode = [](const Value &value) {
  return std::visit(
      [&](auto &&v) -> Value {
        if constexpr (byfxxm_IsDoublePtr(v))
          return Gtag{K, *v};
        else if constexpr (byfxxm_IsDouble(v))
          return Gtag{K, v};
        else
          throw AbstreeException("gcode error");
      },
      value);
};

inline constexpr auto Comma = [](Value &lhs, const Value &rhs) {
  return std::visit(
      [](auto &&l, auto &&r) -> Value {
        if constexpr (byfxxm_IsGroup(l) && byfxxm_IsDouble(r)) {
          l.push_back(r);
          return l;
        } else if constexpr (byfxxm_IsGroup(l) && byfxxm_IsDoublePtr(r)) {
          l.push_back(*r);
          return l;
        } else if constexpr (byfxxm_IsDouble(l) && byfxxm_IsDouble(r))
          return Group({l, r});
        else if constexpr (byfxxm_IsDouble(l) && byfxxm_IsDoublePtr(r))
          return Group({l, *r});
        else if constexpr (byfxxm_IsDoublePtr(l) && byfxxm_IsDouble(r))
          return Group({*l, r});
        else if constexpr (byfxxm_IsDoublePtr(l) && byfxxm_IsDoublePtr(r))
          return Group({*l, *r});
        else
          throw AbstreeException("comma error");
      },
      lhs, rhs);
};

inline constexpr auto Max = [](Value &value) -> Value {
  return std::visit(
      [](auto &&v) -> Value {
        if constexpr (byfxxm_IsGroup(v)) {
          return *std::ranges::max_element(
              v, [](double lhs, double rhs) { return lhs < rhs; });
        } else if constexpr (byfxxm_IsDouble(v))
          return v;
        else if constexpr (byfxxm_IsDoublePtr(v))
          return *v;
        else
          throw AbstreeException("max error");
      },
      value);
};

inline constexpr auto Min = [](Value &value) {
  return std::visit(
      [](auto &&v) -> Value {
        if constexpr (byfxxm_IsGroup(v)) {
          return *std::ranges::min_element(
              v, [](double lhs, double rhs) { return lhs < rhs; });
        } else if constexpr (byfxxm_IsDouble(v))
          return v;
        else if constexpr (byfxxm_IsDoublePtr(v))
          return *v;
        else
          throw AbstreeException("min error");
      },
      value);
};

inline constexpr auto Not = [](const Value &value) {
  return std::visit(
      [](auto &&v) -> Value {
        if constexpr (byfxxm_IsBool(v))
          return Value{!v};
        else
          throw AbstreeException(R"("NOT" error)");
      },
      value);
};

using Gfunc = bool (Ginterface::*)(const Gparams &, const Address *);

struct _GtagHash {
  size_t operator()(const Gtag &tag) const {
    return std::hash<token::Kind>()(tag.code) ^ std::hash<double>()(tag.value);
  }
};

struct _GtagEqual {
  bool operator()(const Gtag &tag1, const Gtag &tag2) const {
    return (tag1.code == tag2.code) && (tag1.value == tag2.value);
  }
};

inline const std::pmr::unordered_map<Gtag, Gfunc, _GtagHash, _GtagEqual>
    gtag_to_ginterface = {
        {{token::Kind::G, 0}, &Ginterface::G0},
        {{token::Kind::G, 1}, &Ginterface::G1},
        {{token::Kind::G, 2}, &Ginterface::G2},
        {{token::Kind::G, 3}, &Ginterface::G3},
        {{token::Kind::G, 4}, &Ginterface::G4},
};

inline constexpr auto Gcmd = [](const std::pmr::vector<Value> &tags,
                                Address *addr, Ginterface *pimpl) -> Value {
  if (!pimpl)
    return {};

  if (tags.empty())
    throw AbstreeException();

  Gparams par{&mempool};
  std::ranges::for_each(tags, [&](const Value &ele) {
    auto tag = std::get<Gtag>(ele);
    if (IsNaN(tag.value) || gtag_to_ginterface.contains(tag))
      return;

    par.push_back(std::get<Gtag>(ele));
  });

  auto iter = std::ranges::find_if(tags, [](auto &&tag) {
    if (!std::holds_alternative<Gtag>(tag))
      throw AbstreeException();

    return gtag_to_ginterface.contains(std::get<Gtag>(tag));
  });

  auto func = iter == tags.end() ? &Ginterface::None
                                 : gtag_to_ginterface.at(std::get<Gtag>(*iter));
  if (!(pimpl->*func)(par, addr))
    throw AbstreeException();

  return std::monostate{};
};
} // namespace predicate

template <class... Ts>
consteval std::variant<std::decay_t<Ts>...> ToVariant(Ts &&...) noexcept {
  return {};
}

// 一元操作符
using Unary = decltype(ToVariant(
    predicate::Neg, predicate::Pos, predicate::Gcode<token::Kind::G>,
    predicate::Gcode<token::Kind::M>, predicate::Gcode<token::Kind::X>,
    predicate::Gcode<token::Kind::Y>, predicate::Gcode<token::Kind::Z>,
    predicate::Gcode<token::Kind::A>, predicate::Gcode<token::Kind::B>,
    predicate::Gcode<token::Kind::C>, predicate::Gcode<token::Kind::I>,
    predicate::Gcode<token::Kind::J>, predicate::Gcode<token::Kind::K>,
    predicate::Gcode<token::Kind::N>, predicate::Gcode<token::Kind::F>,
    predicate::Gcode<token::Kind::S>, predicate::Gcode<token::Kind::O>,
    predicate::Max, predicate::Min, predicate::Not));

// 二元操作符
using Binary = decltype(ToVariant(
    predicate::Plus, predicate::Minus, predicate::Multi, predicate::Div,
    predicate::Assign, predicate::GT, predicate::GE, predicate::LT,
    predicate::LE, predicate::EQ, predicate::NE, predicate::Comma));

using Sharp = decltype(ToVariant(predicate::Sharp));

using Gcmd = decltype(ToVariant(predicate::Gcmd));

// 定义谓词
using Predicate = std::variant<Value, Unary, Binary, Sharp, Gcmd>;
} // namespace byfxxm

#endif
