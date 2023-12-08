#ifndef _BYFXXM_ABSTREE_HPP_
#define _BYFXXM_ABSTREE_HPP_

#include "predicate.hpp"
#include <cassert>
#include <tuple>
#include <variant>

namespace byfxxm {
struct SnapshotHelper {
  const GetSnapshot &get_snapshot;
  const MarkSnapshot &mark_snapshot;
  const GotoSnapshot &goto_snapshot;
  const SnapshotTable &snapshot_table;
};

class Ginterface;
class Abstree {
public:
  struct Node;
  using NodePtr = UniquePtr<Node>;

  struct Node {
    Predicate pred;
    std::pmr::vector<NodePtr> subs{&mempool};
  };

  Abstree(NodePtr &root, Value &rval, Address *addr, Ginterface *gimpl,
          const SnapshotHelper &helper) noexcept
      : _root(&root), _return_val(rval), _addr(addr), _gimpl(gimpl),
        _snapshot_helper(helper) {
    assert(*std::get<NodePtr *>(_root));
  }

  Abstree(NodePtr &&root, Value &rval, Address *addr, Ginterface *gimpl,
          SnapshotHelper helper) noexcept
      : _root(std::move(root)), _return_val(rval), _addr(addr), _gimpl(gimpl),
        _snapshot_helper(helper) {
    assert(std::get<NodePtr>(_root));
  }

  ~Abstree() = default;
  Abstree(const Abstree &) = delete;
  Abstree(Abstree &&) noexcept = default;
  Abstree &operator=(const Abstree &) = delete;
  Abstree &operator=(Abstree &&) noexcept = default;

  Value operator()() const {
    try {
      std::visit(
          Overloaded{
              [this](const NodePtr &root) { _return_val = _Execute(root); },
              [this](const NodePtr *root) { _return_val = _Execute(*root); }},
          _root);
    } catch (const ParseException &ex) {
      throw SyntaxException(_snapshot_helper.get_snapshot().line, ex.what());
    }

    return _return_val;
  }

private:
  Value _Execute(const NodePtr &node) const {
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
                  [&](auto &&func) {
                    return func(params, _addr, _gimpl,
                                _snapshot_helper.mark_snapshot);
                  },
                  gcmd);
            },
            [&](const Goto &goto_) {
              assert(params.size() == 1);
              return std::visit(
                  [&](auto &&func) {
                    return func(params[0], _snapshot_helper.goto_snapshot,
                                _snapshot_helper.snapshot_table);
                  },
                  goto_);
            },
        },
        node->pred);
  }

private:
  std::variant<NodePtr, NodePtr *> _root;
  Value &_return_val;
  Address *_addr{nullptr};
  Ginterface *_gimpl{nullptr};
  SnapshotHelper _snapshot_helper;
};

using Segment = std::tuple<Abstree::NodePtr, Snapshot>;
} // namespace byfxxm

#endif
