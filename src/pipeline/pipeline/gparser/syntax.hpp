#ifndef _BYFXXM_SYNTAX_HPP_
#define _BYFXXM_SYNTAX_HPP_

#include "address.hpp"
#include "block.hpp"
#include "grammar.hpp"
#include "lexer.hpp"
#include "memory.hpp"
#include "production.hpp"

namespace byfxxm {
inline Segment *GetSegment(UniquePtr<block::Block> &block) {
  if (block) {
    auto tree = block->Next();
    if (tree)
      return tree;
    else
      block.Reset();
  }

  return {};
}

using AbstreeTuple = std::tuple<Abstree, size_t>;

template <StreamConcept T> class Syntax {
public:
  Syntax(T &&stream, Address *addr, Ginterface *gimpl)
      : _lex(std::move(stream)), _addr(addr), _gimpl(gimpl) {}

  std::optional<AbstreeTuple> Next() {
    try {
      if (auto seg = GetSegment(_remain_block)) {
        return _ToAbstreeTuple(*seg);
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
        return _ToAbstreeTuple(std::move(stmt.value()));

      return {};
    } catch (const ParseException &ex) {
      throw SyntaxWithLineException(_lineno, ex.what());
    }
  }

private:
  AbstreeTuple _ToAbstreeTuple(Segment &seg) {
    auto &[nodeptr, line] = seg;
    return {Abstree(nodeptr, _return_val, _addr, _gimpl), line};
  }

  AbstreeTuple _ToAbstreeTuple(Segment &&seg) {
    auto &[nodeptr, line] = seg;
    return {Abstree(std::move(nodeptr), _return_val, _addr, _gimpl), line};
  }

  AbstreeTuple _ToAbstreeTuple(Statement &&stmt) {
    return std::visit(
        Overloaded{
            [this](Segment &&seg) -> AbstreeTuple {
              return _ToAbstreeTuple(std::move(seg));
            },
            [this](UniquePtr<block::Block> &&block_) -> AbstreeTuple {
              _remain_block = std::move(block_);
              auto seg = GetSegment(_remain_block);
              assert(seg);
              return _ToAbstreeTuple(*seg);
            },
        },
        std::move(stmt));
  }

private:
  Lexer<T> _lex;
  size_t _lineno{1};
  Value _return_val;
  Address *_addr{nullptr};
  Ginterface *_gimpl{nullptr};
  UniquePtr<block::Block> _remain_block;
};

template <class T> Syntax(T) -> Syntax<T>;
} // namespace byfxxm

#endif
