#ifndef _BYFXXM_LEXER_HPP_
#define _BYFXXM_LEXER_HPP_

#include "exception.hpp"
#include "token.hpp"
#include "word.hpp"
#include <filesystem>
#include <optional>
#include <string>

namespace byfxxm {
inline void SkipSpaces(auto &&stream) {
  while (token::IsSpace(stream.peek())) {
    stream.get();
  }
}

template <StreamConcept T> class Lexer {
public:
  Lexer(T &&stream) : _stream(std::move(stream)) {}

  token::Token Get() {
    _lasttok = Peek();
    _peektok.reset();
    return _lasttok.value();
  }

  token::Token Peek() {
    if (!_peektok.has_value())
      _peektok = _Next();

    return _peektok.value();
  }

private:
  token::Token _Next() {
    SkipSpaces(_stream);
    if (_stream.eof())
      return token::Token{token::Kind::KEOF, nan};

    std::string word;
    word.push_back(_stream.get());

    auto peek = [this]() { return _stream.peek(); };
    auto get = [this]() { return _stream.get(); };
    auto last = [this]() -> const std::optional<token::Token> & {
      return _lasttok;
    };

    for (const auto &elem : word::WordsList::words) {
      std::optional<token::Token> tok;
      if (elem->First(word.front()) &&
          (tok = elem->Rest(word, {peek, get, last})))
        return tok.value();
    }

    throw LexException();
  }

private:
  T _stream;
  std::optional<token::Token> _lasttok;
  std::optional<token::Token> _peektok;
};

template <class T> Lexer(T) -> Lexer<T>;
} // namespace byfxxm

#endif
