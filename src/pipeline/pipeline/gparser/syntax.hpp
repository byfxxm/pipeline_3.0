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

  return nullptr;
}

using AbstreeWithLineno = std::tuple<Abstree, size_t>;

template <StreamConcept T> class Syntax {
public:
  Syntax(T &&stream, Address *addr, Ginterface *pimpl)
      : _lex(std::move(stream)), _addr(addr), _pimpl(pimpl) {}

  std::optional<AbstreeWithLineno> Next() {
    try {
      if (auto seg = GetSegment(_remain_block)) {
        auto &[nodeptr, line] = *seg;
        return AbstreeWithLineno(_ToAbstree(nodeptr), line);
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
  template <class T1>
    requires std::is_same_v<std::decay_t<T1>, Abstree::NodePtr>
  Abstree _ToAbstree(T1 &&nodeptr) {
    return Abstree(std::forward<T1>(nodeptr), _return_val, _addr, _pimpl);
  }

  AbstreeWithLineno _ToAbstreeWithLineno(Statement &&stmt) {
    return std::visit(
        Overloaded{
            [this](Segment &&seg) -> AbstreeWithLineno {
              auto &[nodeptr, line] = seg;
              return {_ToAbstree(std::move(nodeptr)), line};
            },
            [this](UniquePtr<block::Block> &&block_) -> AbstreeWithLineno {
              _remain_block = std::move(block_);
              auto seg = GetSegment(_remain_block);
              assert(seg);
              auto &[nodeptr, line] = *seg;
              return {_ToAbstree(nodeptr), line};
            },
        },
        std::move(stmt));
  }

private:
  Lexer<T> _lex;
  size_t _lineno{1};
  Value _return_val;
  Address *_addr{nullptr};
  Ginterface *_pimpl{nullptr};
  UniquePtr<block::Block> _remain_block;
};

template <class T> Syntax(T) -> Syntax<T>;
} // namespace byfxxm

#endif
