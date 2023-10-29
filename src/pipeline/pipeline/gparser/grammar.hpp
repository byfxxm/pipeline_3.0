#ifndef _GPARSER_GRAMMAR_HPP_
#define _GPARSER_GRAMMAR_HPP_

#include "abstree.hpp"
#include "block.hpp"
#include "memory.hpp"
#include "production.hpp"
#include "token.hpp"

namespace byfxxm {
namespace grammar {
using Get = std::function<token::Token()>;
using Peek = std::function<token::Token()>;
using Line = std::function<size_t()>;

struct Utils {
  Get get;
  Peek peek;
  Line line;
  GetRetVal return_val;
};

constexpr bool IsNewSegment(const token::Token &tok) {
  return tok.kind == token::Kind::NEWLINE || tok.kind == token::Kind::SEMI;
}

constexpr bool IsEndOfFile(const token::Token &tok) {
  return tok.kind == token::Kind::KEOF;
}

inline void SkipNewlines(const Utils &utils) {
  for (;;) {
    auto tok = utils.peek();
    if (tok.kind != token::Kind::NEWLINE)
      break;

    utils.get();
  }
}

inline std::optional<Statement> GetStatement(const Utils &);

class Grammar {
public:
  virtual ~Grammar() = default;
  virtual bool First(const token::Token &) const = 0;
  virtual std::optional<Statement> Rest(Segment &&, const Utils &) const = 0;
};

class Blank : public Grammar {
  virtual bool First(const token::Token &tok) const override {
    return tok.kind == token::Kind::NEWLINE || tok.kind == token::Kind::SEMI;
  }

  virtual std::optional<Statement> Rest(Segment &&,
                                        const Utils &) const override {
    return {};
  }
};

class Expr : public Grammar {
  virtual bool First(const token::Token &tok) const override {
    return tok.kind == token::Kind::SHARP || tok.kind == token::Kind::LB;
  }

  virtual std::optional<Statement> Rest(Segment &&seg,
                                        const Utils &utils) const override {
    for (;;) {
      auto tok = utils.peek();
      if (IsNewSegment(tok))
        break;

      seg.push_back(utils.get());
    }

    return Statement(std::move(seg), utils.line());
  }
};

class Ggram : public Grammar {
  virtual bool First(const token::Token &tok) const override {
    return IsGcode(tok);
  }

  virtual std::optional<Statement> Rest(Segment &&seg,
                                        const Utils &utils) const override {
    Segment gtag{&mempool};
    for (;;) {
      auto tok = utils.peek();
      if (IsNewSegment(tok)) {
        if (!gtag.empty())
          seg.push_back(expr(gtag));
        break;
      }

      if (!IsGcode(tok)) {
        gtag.push_back(utils.get());
        continue;
      }

      if (gtag.empty()) {
        seg.push_back(utils.get());
      } else {
        seg.push_back(expr(gtag));
        gtag.clear();
      }
    }

    Segment ret{&mempool};
    ret.push_back(gtree(seg));
    return Statement(std::move(ret), utils.line());
  }
};

class IfElse : public Grammar {
  virtual bool First(const token::Token &tok) const override {
    return tok.kind == token::Kind::IF;
  }

  virtual std::optional<Statement> Rest(Segment &&seg,
                                        const Utils &utils) const override {
    using If = block::IfElse::If;
    using Else = block::IfElse::Else;

    auto read_cond = [&]() -> Statement {
      Segment seg{&mempool};
      for (;;) {
        auto tok = utils.get();
        if (tok.kind == token::Kind::NEWLINE)
          throw SyntaxException();

        if (tok.kind == token::Kind::THEN)
          break;

        seg.push_back(std::move(tok));
      }

      return {std::move(seg), utils.line()};
    };

    auto read_scope = [&](Scope &scope) {
      for (;;) {
        SkipNewlines(utils);
        auto tok = utils.peek();
        if (tok.kind == token::Kind::ELSE || tok.kind == token::Kind::ELSEIF ||
            tok.kind == token::Kind::ENDIF)
          break;

        auto stmt = GetStatement(utils);
        if (!stmt)
          break;

        scope.push_back(std::move(stmt.value()));
      }
    };

    // read if
    block::IfElse ifelse(utils.return_val);
    ifelse._ifs.push_back(If(read_cond()));
    read_scope(ifelse._ifs.back().scope);

    // read elseif
    for (;;) {
      SkipNewlines(utils);
      auto tok = utils.peek();
      if (tok.kind != token::Kind::ELSEIF)
        break;

      ifelse._ifs.push_back(If(read_cond()));
      read_scope(ifelse._ifs.back().scope);
    }

    // read else
    auto tok = utils.peek();
    if (tok.kind == token::Kind::ELSE) {
      utils.get();
      SkipNewlines(utils);
      read_scope(ifelse._else.scope);
    }

    // endif
    tok = utils.get();
    if (tok.kind != token::Kind::ENDIF)
      throw SyntaxException();

    return Statement(ClonePtr<block::Block>(
                         MakeUnique<block::IfElse>(mempool, std::move(ifelse))),
                     utils.line());
  }
};

class While : public Grammar {
  virtual bool First(const token::Token &tok) const override {
    return tok.kind == token::Kind::WHILE;
  }

  virtual std::optional<Statement> Rest(Segment &&seg,
                                        const Utils &utils) const override {
    auto read_cond = [&]() -> Statement {
      Segment seg{&mempool};
      for (;;) {
        auto tok = utils.get();
        if (tok.kind == token::Kind::NEWLINE)
          throw SyntaxException();

        if (tok.kind == token::Kind::DO)
          break;

        seg.push_back(std::move(tok));
      }

      return {std::move(seg), utils.line()};
    };

    auto read_scope = [&](Scope &scope) {
      for (;;) {
        SkipNewlines(utils);
        auto tok = utils.peek();
        if (tok.kind == token::Kind::END)
          break;

        auto stmt = GetStatement(utils);
        if (!stmt)
          break;

        scope.push_back(std::move(stmt.value()));
      }
    };

    block::While wh(utils.return_val);
    wh._cond = read_cond();
    read_scope(wh._scope);

    // end
    auto tok = utils.get();
    if (tok.kind != token::Kind::END)
      throw SyntaxException();

    return Statement(ClonePtr<block::Block>(
                         MakeUnique<block::While>(mempool, std::move(wh))),
                     utils.line());
  }
};

template <class... _Grams> struct _GrammarsList {
  inline static const std::unique_ptr<grammar::Grammar>
      grammars[sizeof...(_Grams)]{std::make_unique<_Grams>()...};
};

using GrammarsList =
    _GrammarsList<grammar::Blank, grammar::Expr, grammar::Ggram,
                  grammar::IfElse, grammar::While>;

inline std::optional<Statement> GetStatement(const Utils &utils) {
  for (;;) {
    auto tok = utils.peek();
    if (IsEndOfFile(tok))
      return {};

    Segment seg{&mempool};
    seg.push_back(utils.get());

    auto iter = std::begin(GrammarsList::grammars);
    for (; iter != std::end(GrammarsList::grammars); ++iter) {
      if ((*iter)->First(tok)) {
        std::optional<Statement> sub;
        if (!(sub = (*iter)->Rest(std::move(seg), utils)).has_value())
          break;
        return std::move(sub.value());
      }
    }

    if (iter == std::end(GrammarsList::grammars))
      throw SyntaxException();
    else
      assert(dynamic_cast<grammar::Blank *>((*iter).get()));
  }
}
} // namespace grammar
} // namespace byfxxm

#endif
