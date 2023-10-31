#ifndef _BYFXXM_PRODUCTION_HPP_
#define _BYFXXM_PRODUCTION_HPP_

#include "abstree.hpp"
#include "predicate.hpp"
#include "token.hpp"
#include <ranges>
#include <variant>
#include <vector>

namespace byfxxm {
struct TokenTraits {
  static constexpr size_t default_priority = -1;
  size_t priority{default_priority};
  Predicate pred{Value()};
  bool left_to_right{false};
};

inline const std::pmr::unordered_map<token::Kind, TokenTraits> token_traits = {
    {token::Kind::COMMA, {0, Binary{predicate::Comma}, true}},
    {token::Kind::ASSIGN, {1, Binary{predicate::Assign}}},
    {token::Kind::GT, {2, Binary{predicate::GT}}},
    {token::Kind::GE, {2, Binary{predicate::GE}}},
    {token::Kind::LT, {2, Binary{predicate::LT}}},
    {token::Kind::LE, {2, Binary{predicate::LE}}},
    {token::Kind::EQ, {2, Binary{predicate::EQ}}},
    {token::Kind::NE, {2, Binary{predicate::NE}}},
    {token::Kind::PLUS, {3, Binary{predicate::Plus}, true}},
    {token::Kind::MINUS, {3, Binary{predicate::Minus}, true}},
    {token::Kind::MUL, {4, Binary{predicate::Multi}, true}},
    {token::Kind::DIV, {4, Binary{predicate::Div}, true}},
    {token::Kind::SHARP, {5, Sharp{predicate::Sharp}}},
    {token::Kind::NEG, {5, Unary{predicate::Neg}}},
    {token::Kind::POS, {5, Unary{predicate::Pos}}},
    {token::Kind::NOT, {5, Unary{predicate::Not}}},
    {token::Kind::MAX, {5, Unary{predicate::Max}}},
    {token::Kind::MIN, {5, Unary{predicate::Min}}},
    {token::Kind::G, {5, Unary{predicate::Gcode<token::Kind::G>}}},
    {token::Kind::M, {5, Unary{predicate::Gcode<token::Kind::M>}}},
    {token::Kind::X, {5, Unary{predicate::Gcode<token::Kind::X>}}},
    {token::Kind::Y, {5, Unary{predicate::Gcode<token::Kind::Y>}}},
    {token::Kind::Z, {5, Unary{predicate::Gcode<token::Kind::Z>}}},
    {token::Kind::A, {5, Unary{predicate::Gcode<token::Kind::A>}}},
    {token::Kind::B, {5, Unary{predicate::Gcode<token::Kind::B>}}},
    {token::Kind::C, {5, Unary{predicate::Gcode<token::Kind::C>}}},
    {token::Kind::I, {5, Unary{predicate::Gcode<token::Kind::I>}}},
    {token::Kind::J, {5, Unary{predicate::Gcode<token::Kind::J>}}},
    {token::Kind::K, {5, Unary{predicate::Gcode<token::Kind::K>}}},
    {token::Kind::N, {5, Unary{predicate::Gcode<token::Kind::N>}}},
    {token::Kind::F, {5, Unary{predicate::Gcode<token::Kind::F>}}},
    {token::Kind::S, {5, Unary{predicate::Gcode<token::Kind::S>}}},
    {token::Kind::O, {5, Unary{predicate::Gcode<token::Kind::O>}}},
    {token::Kind::CON, {}},
};

using SyntaxNode = std::variant<token::Token, Abstree::NodePtr>;
using SyntaxNodeList = std::pmr::vector<SyntaxNode>;

class Expression {
public:
  Abstree::NodePtr operator()(SyntaxNodeList &synlist) const {
    return _Expression(synlist);
  }

private:
  using _SyntaxNodeListRng =
      decltype(std::ranges::subrange(std::declval<SyntaxNodeList &>()));

  Abstree::NodePtr _Expression(_SyntaxNodeListRng range) const {
    if (range.empty())
      return {};

    SyntaxNodeList synlist = _ProcessBracket(range);
    auto minpri = _FindMinPriority(synlist);

    auto node = _CurNode(*minpri);
    if (auto first = _Expression(_SyntaxNodeListRng(synlist.begin(), minpri)))
      node->subs.push_back(std::move(first));
    if (auto second =
            _Expression(_SyntaxNodeListRng(minpri + 1, synlist.end())))
      node->subs.push_back(std::move(second));

    _CheckError(node);
    return node;
  }

  SyntaxNodeList _ProcessBracket(_SyntaxNodeListRng range) const {
    SyntaxNodeList main{&mempool};
    SyntaxNodeList sub{&mempool};
    int level = 0;
    for (auto &node : range) {
      if (std::holds_alternative<Abstree::NodePtr>(node)) {
        main.push_back(std::move(node));
        continue;
      }

      auto tok = std::get<token::Token>(node);
      if (tok.kind == token::Kind::LB) {
        ++level;
        if (level == 1)
          continue;
      } else if (tok.kind == token::Kind::RB) {
        --level;
        if (level == 0) {
          main.push_back(_Expression(sub));
          sub.clear();
          continue;
        }
      }

      if (level > 0)
        sub.push_back(std::move(tok));
      else if (level == 0)
        main.push_back(std::move(tok));
      else
        throw SyntaxException();
    }

    if (level != 0)
      throw SyntaxException();

    return main;
  }

  SyntaxNodeList::iterator _FindMinPriority(_SyntaxNodeListRng range) const {
    auto less = [](const SyntaxNode &lhs, const SyntaxNode &rhs) {
      size_t lhs_pri = TokenTraits::default_priority;
      size_t rhs_pri = TokenTraits::default_priority;

      if (auto p = std::get_if<token::Token>(&lhs))
        lhs_pri = token_traits.at(p->kind).priority;
      if (auto p = std::get_if<token::Token>(&rhs))
        rhs_pri = token_traits.at(p->kind).priority;

      return lhs_pri < rhs_pri;
    };

    auto ret = std::ranges::min_element(range, less);
    if (auto p = std::get_if<token::Token>(&*ret);
        p && (token_traits.at(p->kind).left_to_right))
      ret = std::ranges::min_element(range | std::views::reverse, less).base() -
            1;

    return ret;
  }

  Abstree::NodePtr _CurNode(SyntaxNode &node) const {
    auto ret = ClonePtr(MakeUnique<Abstree::Node>(mempool));
    if (auto abs = std::get_if<Abstree::NodePtr>(&node)) {
      ret = std::move(*abs);
    } else {
      auto &tok = std::get<token::Token>(node);
      if (!tok.value)
        ret->pred = token_traits.at(tok.kind).pred;
      else
        ret->pred = std::visit([](auto &&v) -> Value { return v; },
                               std::move(tok.value.value()));
    }

    return ret;
  }

  void _CheckError(const Abstree::NodePtr &node) const {
    assert(node);
    std::visit(Overloaded{
                   [&](const Value &value) {
                     if (std::holds_alternative<std::monostate>(value))
                       return;

                     if (!node->subs.empty())
                       throw SyntaxException();
                   },
                   [&](const Unary &) {
                     if (node->subs.size() != 1)
                       throw SyntaxException();
                   },
                   [&](const Binary &) {
                     if (node->subs.size() != 2)
                       throw SyntaxException();
                   },
                   [&](const Sharp &) {
                     if (node->subs.size() != 1)
                       throw SyntaxException();
                   },
                   [&](const Gcmd &) {
                     if (node->subs.size() == 0)
                       throw SyntaxException();
                   },
                   [](const auto &) { // default
                     throw SyntaxException();
                   },
               },
               node->pred);
  }
};

class Gtree {
public:
  Abstree::NodePtr operator()(SyntaxNodeList &synlist) const {
    if (synlist.empty() || (synlist.size() & 0x1) != 0)
      throw SyntaxException();

    auto root = MakeUnique<Abstree::Node>(mempool);
    root->pred = Gcmd{};
    for (auto iter = synlist.begin(); iter != synlist.end();) {
      auto node = MakeUnique<Abstree::Node>(mempool);
      node->pred = _TokToPred(std::get<token::Token>(*iter++));
      node->subs.push_back(std::move(std::get<Abstree::NodePtr>(*iter++)));
      root->subs.push_back(std::move(node));
    }

    return root;
  }

private:
  Predicate _TokToPred(const token::Token &tok) const {
    if (!token_traits.contains(tok.kind))
      throw SyntaxException();

    return token_traits.at(tok.kind).pred;
  }
};

inline constexpr Expression expr;
inline constexpr Gtree gtree;
} // namespace byfxxm

#endif
