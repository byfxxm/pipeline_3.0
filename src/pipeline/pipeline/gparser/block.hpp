﻿#ifndef _BYFXXM_BLOCK_HPP_
#define _BYFXXM_BLOCK_HPP_

#include "memory.hpp"
#include "production.hpp"
#include <optional>
#include <tuple>

namespace byfxxm {
namespace grammar {
class IfElse;
class While;
} // namespace grammar

namespace block {
class Block;
}

using Segment = std::tuple<Abstree::NodePtr, size_t>;
using Statement = std::variant<Segment, UniquePtr<block::Block>>;
using Scope = std::pmr::vector<Statement>;

namespace block {
class Block {
public:
  virtual ~Block() = default;
  virtual Segment *Next() = 0;
};

inline Segment *GetStatement(Scope &scope, size_t &index) {
  if (index == scope.size())
    return nullptr;

  return std::visit(Overloaded{[&](Segment &seg) {
                                 ++index;
                                 return &seg;
                               },
                               [&](UniquePtr<Block> &block) {
                                 auto seg = block->Next();
                                 if (!seg) {
                                   seg = GetStatement(scope, ++index);
                                 }
                                 return seg;
                               }},
                    scope[index]);
};

class IfElse : public Block {
  struct If {
    Segment cond;
    Scope scope{&mempool};
  };

  struct Else {
    Scope scope{&mempool};
  };

  IfElse(GetRetVal get_ret) : _get_ret(get_ret) {}

  virtual Segment *Next() override {
    if (_iscond) {
      if (_cur_stmt > 0 && std::get<bool>(_get_ret())) {
        --_cur_stmt;
        _iscond = false;
        return GetStatement(_ifs[_cur_stmt].scope, _scope_index);
      }

      if (_cur_stmt == 0)
        return &_ifs[_cur_stmt++].cond;

      if (_cur_stmt == _ifs.size()) {
        _iscond = false;
        return GetStatement(_else.scope, _scope_index);
      }

      if (!std::holds_alternative<bool>(_get_ret()))
        throw SyntaxException();

      auto cond = std::get<bool>(_get_ret());
      if (cond) {
        _iscond = false;
        return GetStatement(_ifs[_cur_stmt].scope, _scope_index);
      }

      return &_ifs[_cur_stmt++].cond;
    }

    if (_cur_stmt == _ifs.size())
      return GetStatement(_else.scope, _scope_index);

    if (_scope_index == _ifs[_cur_stmt].scope.size())
      return {};

    return GetStatement(_ifs[_cur_stmt].scope, _scope_index);
  }

  std::pmr::vector<If> _ifs{&mempool};
  Else _else;
  size_t _cur_stmt{0};
  bool _iscond{true};
  GetRetVal _get_ret;
  size_t _scope_index{0};
  friend class grammar::IfElse;
};

class While : public Block {
  While(GetRetVal get_ret) : _get_ret(get_ret) {}

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
      if (!std::holds_alternative<bool>(_get_ret()))
        throw SyntaxException();

      auto cond = std::get<bool>(_get_ret());
      if (!cond)
        return {};
    }

    _iscond = false;
    return GetStatement(_scope, _scope_index);
  }

  Segment _cond;
  Scope _scope{&mempool};
  bool _iscond{true};
  GetRetVal _get_ret;
  size_t _scope_index{0};
  friend class grammar::While;
};
} // namespace block
} // namespace byfxxm

#endif
