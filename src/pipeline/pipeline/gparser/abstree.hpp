#ifndef _GPARSER_ABSTREE_HPP_
#define _GPARSER_ABSTREE_HPP_

#include "memory.hpp"
#include "predicate.hpp"
#include "token.hpp"
#include <cassert>
#include <variant>

namespace byfxxm {
class Ginterface;

class Abstree {
public:
  struct Node;
  using NodePtr = ClonePtr<Node>;

  struct Node {
    Predicate pred;
    std::pmr::vector<NodePtr> subs{&mempool};
  };

  Abstree(NodePtr &&root, Value &rval, Address *addr,
          Ginterface *pimpl) noexcept
      : _root(std::move(root)), _return_val(&rval), _addr(addr), _pimpl(pimpl) {
    assert(_root);
  }

  ~Abstree() = default;
  Abstree(const Abstree &) = delete;
  Abstree(Abstree &&) noexcept = default;
  Abstree &operator=(const Abstree &) = delete;
  Abstree &operator=(Abstree &&) noexcept = default;

  Value Execute() {
    *_return_val = _Execute(_root);
    return *_return_val;
  }

private:
  Value _Execute(const NodePtr &node) {
    if (std::holds_alternative<Value>(node->pred))
      return std::get<Value>(node->pred);

    std::pmr::vector<Value> params{&mempool};
    std::ranges::for_each(node->subs,
                          [&](auto &&p) { params.push_back(_Execute(p)); });

    return std::visit(
        Overloaded{
            [](const Value &value) { return value; },
            [&](const Unary &unary) {
              assert(params.size() == 1);
              return std::visit([&](auto &&func) { return func(params[0]); },
                                unary);
            },
            [&](const Binary &binary) {
              assert(params.size() == 2);
              return std::visit(
                  [&](auto &&func) { return func(params[0], params[1]); },
                  binary);
            },
            [&](const Sharp &sharp) {
              assert(params.size() == 1);
              return std::visit(
                  [&](auto &&func) { return func(params[0], _addr); }, sharp);
            },
            [&](const Gcmd &gcmd) {
              assert(!params.empty());
              return std::visit(
                  [&](auto &&func) { return func(params, _addr, _pimpl); },
                  gcmd);
            },
            [](const auto &) -> Value { // default
              throw AbstreeException();
            },
        },
        node->pred);
  }

private:
  NodePtr _root;
  Value *_return_val{nullptr};
  Address *_addr{nullptr};
  Ginterface *_pimpl{nullptr};
};
} // namespace byfxxm

#endif
