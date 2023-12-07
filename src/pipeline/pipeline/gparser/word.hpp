#ifndef _BYFXXM_WORD_HPP_
#define _BYFXXM_WORD_HPP_

#include <functional>

namespace byfxxm {
namespace word {
using Peek = std::function<char()>;
using Get = std::function<char()>;
using Last = std::function<const std::optional<token::Token> &()>;

struct Utils {
  Peek peek;
  Get get;
  Last last;
};

class Word {
public:
  virtual ~Word() = default;
  virtual bool First(char) const = 0;
  virtual std::optional<token::Token> Rest(std::string &,
                                           const Utils &) const = 0;
};

class Sharp : public Word {
  virtual bool First(char ch) const override { return token::IsSharp(ch); }

  virtual std::optional<token::Token> Rest(std::string &word,
                                           const Utils &utils) const override {
    return token::Token{token::Kind::SHARP, {}};
  }
};

class Constant : public Word {
  virtual bool First(char ch) const override { return isdigit(ch); }

  virtual std::optional<token::Token> Rest(std::string &word,
                                           const Utils &utils) const override {
    for (;;) {
      auto ch = utils.peek();
      if (!std::isdigit(ch) && ch != '.')
        break;

      word.push_back(utils.get());
    }

    return token::Token{token::Kind::CON, std::stod(word)};
  }
};

class Key : public Word {
  virtual bool First(char ch) const override { return token::IsKeyword(ch); }

  virtual std::optional<token::Token> Rest(std::string &word,
                                           const Utils &utils) const override {
    for (;;) {
      auto ch = utils.peek();
      if (!std::isalpha(ch))
        break;

      word.push_back(utils.get());
    }

    if (!token::IsKeyword(word))
      return {};

    return token::Token{token::keywords.at(word), {}};
  }
};

class Symbol : public Word {
  virtual bool First(char ch) const override { return token::IsSymbol(ch); }

  virtual std::optional<token::Token> Rest(std::string &word,
                                           const Utils &utils) const override {
    if (!token::symbols.contains(word))
      throw LexException();

    auto sym = token::symbols.at(word);
    auto last_ = utils.last();
    if (sym == token::Kind::PLUS && last_.has_value() &&
        last_.value().kind != token::Kind::CON &&
        last_.value().kind != token::Kind::RB)
      sym = token::Kind::POS;
    else if (sym == token::Kind::MINUS && last_.has_value() &&
             last_.value().kind != token::Kind::CON &&
             last_.value().kind != token::Kind::RB)
      sym = token::Kind::NEG;

    return token::Token{sym, {}};
  }
};

class Gcode : public Word {
  virtual bool First(char ch) const override { return token::IsGcode(ch); }

  virtual std::optional<token::Token> Rest(std::string &word,
                                           const Utils &utils) const override {
    return token::Token{token::gcodes.at(word), nan};
  }
};

class Newline : public Word {
  virtual bool First(char ch) const override { return token::IsNewline(ch); }

  virtual std::optional<token::Token> Rest(std::string &word,
                                           const Utils &utils) const override {
    return token::Token{token::Kind::NEWLINE, {}};
  }
};

template <class... _Words> struct _WordsList {
  inline static const std::unique_ptr<word::Word> words[sizeof...(_Words)]{
      std::make_unique<_Words>()...};
};

using WordsList = _WordsList<word::Sharp, word::Constant, word::Key,
                             word::Symbol, word::Gcode, word::Newline>;
} // namespace word
} // namespace byfxxm

#endif
