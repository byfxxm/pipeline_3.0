#ifndef _BYFXXM_BLOCK_HPP_
#define _BYFXXM_BLOCK_HPP_

#include "abstree.hpp"
#include <optional>

namespace byfxxm {
namespace grammar {
class IfElse;
class While;
} // namespace grammar

namespace block {
class Block {
public:
  virtual ~Block() = default;
  virtual Segment *Next() = 0;
};
} // namespace block

using Statement = std::variant<Segment, UniquePtr<block::Block>>;
using Scope = std::pmr::vector<Statement>;

inline Segment *GetSegment(Scope &scope, size_t &index) {
  if (index == scope.size())
    return {};

  return std::visit(Overloaded{[&](Segment &seg) {
                                 ++index;
                                 return &seg;
                               },
                               [&](UniquePtr<block::Block> &block) {
                                 auto seg = block->Next();
                                 if (!seg)
                                   seg = GetSegment(scope, ++index);

                                 return seg;
                               }},
                    scope[index]);
};

namespace block {
class IfElse : public Block {
  struct If {
    Segment cond;
    Scope scope{&mempool};
  };

  struct Else {
    Scope scope{&mempool};
  };

  IfElse(GetRetVal func) : _get_ret_val_func(std::move(func)) {}

  virtual Segment *Next() override {
    if (_iscond) {
      if (_cur_if > 0 && std::get<bool>(_get_ret_val_func())) {
        --_cur_if;
        _iscond = false;
        return GetSegment(_ifs[_cur_if].scope, _scope_index);
      }

      if (_cur_if == 0)
        return &_ifs[_cur_if++].cond;

      if (_cur_if == _ifs.size()) {
        _iscond = false;
        return GetSegment(_else.scope, _scope_index);
      }

      if (!std::holds_alternative<bool>(_get_ret_val_func()))
        throw SyntaxException();

      auto cond = std::get<bool>(_get_ret_val_func());
      if (cond) {
        _iscond = false;
        return GetSegment(_ifs[_cur_if].scope, _scope_index);
      }

      return &_ifs[_cur_if++].cond;
    }

    if (_cur_if == _ifs.size())
      return GetSegment(_else.scope, _scope_index);

    if (_scope_index == _ifs[_cur_if].scope.size())
      return {};

    return GetSegment(_ifs[_cur_if].scope, _scope_index);
  }

  std::pmr::vector<If> _ifs{&mempool};
  Else _else;
  size_t _cur_if{0};
  bool _iscond{true};
  GetRetVal _get_ret_val_func;
  size_t _scope_index{0};
  friend class grammar::IfElse;
};

class While : public Block {
  While(GetRetVal func) : _get_ret_val_func(std::move(func)) {}

  virtual Segment *Next() override {
    if (_iscond) {
      _iscond = false;
      return &_cond;
    }

    if (_scope_index == _scope.size()) {
      _scope_index = 0;
      return &_cond;
    }

    if (_scope_index == 0) {
      if (!std::holds_alternative<bool>(_get_ret_val_func()))
        throw SyntaxException();

      auto cond = std::get<bool>(_get_ret_val_func());
      if (!cond)
        return {};
    }

    _iscond = false;
    return GetSegment(_scope, _scope_index);
  }

  Segment _cond;
  Scope _scope{&mempool};
  bool _iscond{true};
  GetRetVal _get_ret_val_func;
  size_t _scope_index{0};
  friend class grammar::While;
};
} // namespace block
} // namespace byfxxm

#endif
