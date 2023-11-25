#ifndef _BYFXXM_ABSTREE_HPP_
#define _BYFXXM_ABSTREE_HPP_

#include "predicate.hpp"
#include <cassert>
#include <tuple>
#include <variant>

namespace byfxxm {
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
          const MarkSnapshot &mark_snapshot, const GotoSnapshot &goto_snapshot,
          const SnapshotTable &snapshot_table) noexcept
      : _root(&root), _return_val(&rval), _addr(addr), _gimpl(gimpl),
        _mark_snapshot(mark_snapshot), _goto_snapshot(goto_snapshot),
        _snapshot_table(snapshot_table) {
    assert(*std::get<NodePtr *>(_root));
  }

  Abstree(NodePtr &&root, Value &rval, Address *addr, Ginterface *gimpl,
          const MarkSnapshot &mark_snapshot, const GotoSnapshot &goto_snapshot,
          const SnapshotTable &snapshot_table) noexcept
      : _root(std::move(root)), _return_val(&rval), _addr(addr), _gimpl(gimpl),
        _mark_snapshot(mark_snapshot), _goto_snapshot(goto_snapshot),
        _snapshot_table(snapshot_table) {
    assert(std::get<NodePtr>(_root));
  }

  ~Abstree() = default;
  Abstree(const Abstree &) = delete;
  Abstree(Abstree &&) noexcept = default;
  Abstree &operator=(const Abstree &) = delete;
  Abstree &operator=(Abstree &&) noexcept = default;

  Value operator()() const {
    std::visit(
        Overloaded{
            [this](const NodePtr &root) { *_return_val = _Execute(root); },
            [this](const NodePtr *root) { *_return_val = _Execute(*root); }},
        _root);

    return *_return_val;
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
                    return func(params, _addr, _gimpl, _mark_snapshot);
                  },
                  gcmd);
            },
            [&](const Goto &goto_) {
              assert(params.size() == 1);
              return std::visit(
                  [&](auto &&func) {
                    return func(params[0], _goto_snapshot, _snapshot_table);
                  },
                  goto_);
            },
            [](const auto &) -> Value { // default
              throw AbstreeException();
            },
        },
        node->pred);
  }

private:
  std::variant<NodePtr, NodePtr *> _root;
  Value *_return_val{nullptr};
  Address *_addr{nullptr};
  Ginterface *_gimpl{nullptr};
  const MarkSnapshot &_mark_snapshot;
  const GotoSnapshot &_goto_snapshot;
  const SnapshotTable &_snapshot_table;
};

using Segment = std::tuple<Abstree::NodePtr, Snapshot>;
} // namespace byfxxm

#endif
