﻿#ifndef _GPARSER_SYNTAX_HPP_
#define _GPARSER_SYNTAX_HPP_

#include "address.hpp"
#include "block.hpp"
#include "grammar.hpp"
#include "lexer.hpp"
#include "memory.hpp"
#include "production.hpp"

namespace byfxxm {
inline std::optional<Statement> GetStatement(ClonePtr<block::Block> &block) {
  if (block) {
    auto tree = block->Next();
    if (tree.has_value())
      return std::move(tree.value());
    else
      block.Reset();
  }

  return {};
}

using AbstreeWithLineno = std::tuple<Abstree, size_t>;

template <StreamConcept T> class Syntax {
public:
  Syntax(T &&stream, Address *addr, Ginterface *pimpl)
      : _lex(std::move(stream)), _addr(addr), _pimpl(pimpl) {}

  std::optional<AbstreeWithLineno> Next() {
    try {
      if (auto stmt = GetStatement(_remain_block)) {
        assert(std::holds_alternative<Abstree::NodePtr>(
            std::get<0>(stmt.value())));
        return AbstreeWithLineno{_ToAbstree(std::get<Abstree::NodePtr>(
                                     std::move(std::get<0>(stmt.value())))),
                                 std::get<1>(stmt.value())};
      }

      auto get = [this]() {
        auto tok = _lex.Get();
        if (tok.kind == token::Kind::NEWLINE)
          ++_lineno;
        return tok;
      };
      auto peek = [this]() { return _lex.Peek(); };
      auto line = [this]() { return _lineno; };
      auto get_rval = [this]() -> Value { return _return_val; };

      if (auto stmt = GetStatement(grammar::Utils{get, peek, line, get_rval}))
        return _ToAbstreeWithLineno(std::move(stmt.value()));

      return {};
    } catch (const ParseException &ex) {
      throw SyntaxWithLineException(_lineno, ex.what());
    }
  }

private:
  Abstree _ToAbstree(Abstree::NodePtr &&nodeptr) {
    return Abstree(std::move(nodeptr), _return_val, _addr, _pimpl);
  }

  AbstreeWithLineno _ToAbstreeWithLineno(Statement &&stmt) {
    return std::visit(
        Overloaded{
            [this, line = std::get<1>(stmt)](
                Abstree::NodePtr &&nodeptr) -> AbstreeWithLineno {
              return {_ToAbstree(std::move(nodeptr)), line};
            },
            [this](ClonePtr<block::Block> &&block_) -> AbstreeWithLineno {
              _remain_block = std::move(block_);
              auto stmt_ = GetStatement(_remain_block);
              assert(std::holds_alternative<Abstree::NodePtr>(
                  std::get<0>(stmt_.value())));
              return {_ToAbstree(std::get<Abstree::NodePtr>(
                          std::move(std::get<0>(stmt_.value())))),
                      std::get<1>(stmt_.value())};
            },
        },
        std::move(std::get<0>(stmt)));
  }

private:
  Lexer<T> _lex;
  size_t _lineno{1};
  Value _return_val;
  Address *_addr{nullptr};
  Ginterface *_pimpl{nullptr};
  ClonePtr<block::Block> _remain_block;
};

template <class T> Syntax(T) -> Syntax<T>;
} // namespace byfxxm

#endif
