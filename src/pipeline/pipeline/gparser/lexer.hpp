#ifndef _BYFXXM_LEXER_HPP_
#define _BYFXXM_LEXER_HPP_

#include "exception.hpp"
#include "token.hpp"
#include "word.hpp"
#include <filesystem>
#include <optional>
#include <string>

namespace byfxxm {
inline void SkipSpaces(auto &&peek, auto &&get) {
  while (token::IsSpace(peek())) {
    get();
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

  auto Tellg() const { return _pos; }

  void Seekg(int64_t pos) {
    _stream.seekg(pos);
    _pos = pos;
    _peektok.reset();
  }

  auto BackToBeginningOfLine() {
    while (_pos > 0) {
      auto ch = _stream.unget().peek();
      if (_pos == '\n') {
        _stream.get();
        break;
      }

      --_pos;
    }

    _peektok.reset();
    return _pos;
  }

private:
  token::Token _Next() {
    auto peek = [this]() { return _stream.peek(); };
    auto get = [this]() {
      auto ret = _stream.get();
      _pos += (ret == '\n' ? 2 : 1);
      return ret;
    };
    auto last = [this]() -> const std::optional<token::Token> & {
      return _lasttok;
    };

    SkipSpaces(peek, get);
    if (_stream.eof())
      return token::Token{token::Kind::KEOF, nan};

    std::string word;
    word.push_back(get());

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
  int64_t _pos{0};
};

template <class T> Lexer(T) -> Lexer<T>;
} // namespace byfxxm

#endif
