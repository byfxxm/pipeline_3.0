#ifndef _BYFXXM_PREDICATE_HPP_
#define _BYFXXM_PREDICATE_HPP_

#include "address.hpp"
#include "common.hpp"
#include "exception.hpp"
#include "ginterface.hpp"
#include <algorithm>
#include <ranges>
#include <string>
#include <type_traits>
#include <variant>

#define byfxxm_IsType(v, type) (std::is_same_v<std::decay_t<decltype(v)>, type>)
#define byfxxm_IsDouble(v) byfxxm_IsType(v, double)
#define byfxxm_IsString(v) byfxxm_IsType(v, std::string)
#define byfxxm_IsGroup(v) byfxxm_IsType(v, Group)
#define byfxxm_IsBool(v) byfxxm_IsType(v, bool)
#define byfxxm_IsSharpValue(v) byfxxm_IsType(v, byfxxm::SharpValue)
#define byfxxm_CanConvertToDouble(v)                                           \
  (byfxxm_IsDouble(v) || byfxxm_IsSharpValue(v))

namespace byfxxm {
namespace predicate {
struct Plus {
  auto operator()(const Value &lhs, const Value &rhs) const {
    return std::visit(
        [](auto &&l, auto &&r) -> Value {
          if constexpr (byfxxm_CanConvertToDouble(l) &&
                        byfxxm_CanConvertToDouble(r))
            return Value{l + r};
          else if constexpr (byfxxm_IsString(l) && byfxxm_IsString(r))
            return Value{l + r};
          else
            throw AbstreeException("plus error");
        },
        lhs, rhs);
  }
};

struct Minus {
  auto operator()(const Value &lhs, const Value &rhs) const {
    return std::visit(
        [](auto &&l, auto &&r) -> Value {
          if constexpr (byfxxm_CanConvertToDouble(l) &&
                        byfxxm_CanConvertToDouble(r))
            return Value{l - r};
          else
            throw AbstreeException("minus error");
        },
        lhs, rhs);
  }
};

struct Multi {
  auto operator()(const Value &lhs, const Value &rhs) const {
    return std::visit(
        [](auto &&l, auto &&r) -> Value {
          if constexpr (byfxxm_CanConvertToDouble(l) &&
                        byfxxm_CanConvertToDouble(r))
            return Value{l * r};
          else
            throw AbstreeException("multiple error");
        },
        lhs, rhs);
  }
};

struct Div {
  auto operator()(const Value &lhs, const Value &rhs) const {
    return std::visit(
        [](auto &&l, auto &&r) -> Value {
          if constexpr (byfxxm_CanConvertToDouble(l) &&
                        byfxxm_CanConvertToDouble(r))
            return Value{l / r};
          else
            throw AbstreeException("divide error");
        },
        lhs, rhs);
  }
};

struct Assign {
  auto operator()(Value &lhs, const Value &rhs) const {
    return std::visit(
        [](auto &&l, auto &&r) -> Value {
          if constexpr (byfxxm_IsSharpValue(l) && byfxxm_CanConvertToDouble(r))
            l = +r;
          else
            throw AbstreeException("assign error");

          return Value{l};
        },
        lhs, rhs);
  }
};

struct Neg {
  auto operator()(const Value &value) const {
    return std::visit(
        [](auto &&v) -> Value {
          if constexpr (byfxxm_CanConvertToDouble(v))
            return Value{-v};
          else
            throw AbstreeException("negative error");
        },
        value);
  }
};

struct Pos {
  auto operator()(const Value &value) const {
    return std::visit(
        [](auto &&v) -> Value {
          if constexpr (byfxxm_CanConvertToDouble(v))
            return Value{+v};
          else
            throw AbstreeException("positive error");
        },
        value);
  }
};

struct Sharp {
  auto operator()(const Value &value, Address *addr) const {
    if (!addr)
      throw AbstreeException();

    return std::visit(
        [&](auto &&v) -> Value {
          if constexpr (byfxxm_CanConvertToDouble(v))
            return (*addr)[v];
          else
            throw AbstreeException("sharp error");
        },
        value);
  }
};

struct GT {
  auto operator()(const Value &lhs, const Value &rhs) const {
    return std::visit(
        [](auto &&l, auto &&r) -> Value {
          if constexpr (byfxxm_CanConvertToDouble(l) &&
                        byfxxm_CanConvertToDouble(r))
            return Value{l > r};
          else
            throw AbstreeException();
        },
        lhs, rhs);
  }
};

struct GE {
  auto operator()(const Value &lhs, const Value &rhs) const {
    return std::visit(
        [](auto &&l, auto &&r) -> Value {
          if constexpr (byfxxm_CanConvertToDouble(l) &&
                        byfxxm_CanConvertToDouble(r))
            return Value{l >= r};
          else
            throw AbstreeException();
        },
        lhs, rhs);
  }
};

struct LT {
  auto operator()(const Value &lhs, const Value &rhs) const {
    return std::visit(
        [](auto &&l, auto &&r) -> Value {
          if constexpr (byfxxm_CanConvertToDouble(l) &&
                        byfxxm_CanConvertToDouble(r))
            return Value{l < r};
          else
            throw AbstreeException();
        },
        lhs, rhs);
  }
};

struct LE {
  auto operator()(const Value &lhs, const Value &rhs) const {
    return std::visit(
        [](auto &&l, auto &&r) -> Value {
          if constexpr (byfxxm_CanConvertToDouble(l) &&
                        byfxxm_CanConvertToDouble(r))
            return Value{l <= r};
          else
            throw AbstreeException();
        },
        lhs, rhs);
  }
};

struct EQ {
  auto operator()(const Value &lhs, const Value &rhs) const {
    return std::visit(
        [](auto &&l, auto &&r) -> Value {
          if constexpr (byfxxm_CanConvertToDouble(l) &&
                        byfxxm_CanConvertToDouble(r))
            return Value{l == r};
          else
            throw AbstreeException();
        },
        lhs, rhs);
  }
};

struct NE {
  auto operator()(const Value &lhs, const Value &rhs) const {
    return std::visit(
        [](auto &&l, auto &&r) -> Value {
          if constexpr (byfxxm_CanConvertToDouble(l) &&
                        byfxxm_CanConvertToDouble(r))
            return Value{l != r};
          else
            throw AbstreeException();
        },
        lhs, rhs);
  }
};

template <token::Kind Tok> struct Gcode {
  auto operator()(const Value &value) const {
    return std::visit(
        [](auto &&v) -> Value {
          if constexpr (byfxxm_CanConvertToDouble(v))
            return Gtag{Tok, v};
          else
            throw AbstreeException("gcode error");
        },
        value);
  }
};

struct Comma {
  auto operator()(Value &lhs, const Value &rhs) const {
    return std::visit(
        [](auto &&l, auto &&r) -> Value {
          if constexpr (byfxxm_IsGroup(l) && byfxxm_CanConvertToDouble(r)) {
            l.push_back(r);
            return l;
          } else if constexpr (byfxxm_CanConvertToDouble(l) &&
                               byfxxm_CanConvertToDouble(r))
            return Group({l, r});
          else
            throw AbstreeException("comma error");
        },
        lhs, rhs);
  }
};

struct Max {
  auto operator()(Value &value) const {
    return std::visit(
        [](auto &&v) -> Value {
          if constexpr (byfxxm_IsGroup(v))
            return *std::ranges::max_element(v);
          else if constexpr (byfxxm_CanConvertToDouble(v))
            return v;
          else
            throw AbstreeException("max error");
        },
        value);
  }
};

struct Min {
  auto operator()(Value &value) const {
    return std::visit(
        [](auto &&v) -> Value {
          if constexpr (byfxxm_IsGroup(v))
            return *std::ranges::min_element(v);
          else if constexpr (byfxxm_CanConvertToDouble(v))
            return v;
          else
            throw AbstreeException("min error");
        },
        value);
  }
};

struct Not {
  auto operator()(const Value &value) const {
    return std::visit(
        [](auto &&v) -> Value {
          if constexpr (byfxxm_IsBool(v))
            return Value{!v};
          else
            throw AbstreeException(R"("NOT" error)");
        },
        value);
  }
};

struct Goto {
  auto operator()(const Value &value, const GotoSnapshot &goto_snapshot,
                  const SnapshotTable &table) const {
    return std::visit(
        [&](auto &&v) -> Value {
          double val{};
          if constexpr (byfxxm_CanConvertToDouble(v)) {
            val = v;
          } else
            throw AbstreeException("goto error");

          auto iter = table.find(val);
          if (iter == table.end())
            throw AbstreeException("goto fail");

          goto_snapshot(iter->second);
          return {};
        },
        value);
  }
};

using Gfunc = void (Ginterface::*)(const Ginterface::Utils &);

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
    gtag_to_ginterface = {{{token::Kind::G, 0}, &Ginterface::G0},
                          {{token::Kind::G, 1}, &Ginterface::G1},
                          {{token::Kind::G, 2}, &Ginterface::G2},
                          {{token::Kind::G, 3}, &Ginterface::G3},
                          {{token::Kind::G, 4}, &Ginterface::G4},
                          {{token::Kind::N}, &Ginterface::N}};

struct Gcmd {
  auto operator()(const std::pmr::vector<Value> &tags, Address *addr,
                  Ginterface *gimpl, const MarkSnapshot &mark_snapshot) const
      -> Value {
    if (!gimpl)
      return {};

    if (tags.empty())
      throw AbstreeException();

    Ginterface::Params params{&mempool};
    auto cmds = tags | std::views::filter([&](const Value &elem) {
                  auto tag = std::get<Gtag>(elem);
                  if (gtag_to_ginterface.contains(tag) ||
                      gtag_to_ginterface.contains(Gtag{tag.code})) {
                    return true;
                  }

                  params.push_back(tag);
                  return false;
                });

    std::ranges::for_each(cmds, [&](auto &&elem) {
      auto tag = std::get<Gtag>(elem);
      auto func = gtag_to_ginterface.at(
          gtag_to_ginterface.contains(Gtag{tag.code}) ? Gtag{tag.code} : tag);
      std::invoke(func, gimpl, {tag.value, params, addr, mark_snapshot});
    });

    return {};
  }
};
} // namespace predicate

// 一元操作符
using Unary = std::variant<
    predicate::Neg, predicate::Pos, predicate::Gcode<token::Kind::G>,
    predicate::Gcode<token::Kind::M>, predicate::Gcode<token::Kind::X>,
    predicate::Gcode<token::Kind::Y>, predicate::Gcode<token::Kind::Z>,
    predicate::Gcode<token::Kind::A>, predicate::Gcode<token::Kind::B>,
    predicate::Gcode<token::Kind::C>, predicate::Gcode<token::Kind::I>,
    predicate::Gcode<token::Kind::J>, predicate::Gcode<token::Kind::K>,
    predicate::Gcode<token::Kind::N>, predicate::Gcode<token::Kind::F>,
    predicate::Gcode<token::Kind::S>, predicate::Gcode<token::Kind::O>,
    predicate::Max, predicate::Min, predicate::Not>;

// 二元操作符
using Binary = std::variant<predicate::Plus, predicate::Minus, predicate::Multi,
                            predicate::Div, predicate::Assign, predicate::GT,
                            predicate::GE, predicate::LT, predicate::LE,
                            predicate::EQ, predicate::NE, predicate::Comma>;

// #变量
using Sharp = std::variant<predicate::Sharp>;

// G指令
using Gcmd = std::variant<predicate::Gcmd>;

// GOTO
using Goto = std::variant<predicate::Goto>;

// 定义谓词
using Predicate = std::variant<Value, Unary, Binary, Sharp, Gcmd, Goto>;
} // namespace byfxxm

#endif
