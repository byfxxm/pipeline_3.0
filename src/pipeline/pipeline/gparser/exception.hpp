#ifndef _BYFXXM_EXCEPTION_HPP_
#define _BYFXXM_EXCEPTION_HPP_

#include <exception>
#include <format>
#include <string>

namespace byfxxm {
class ParseException : public std::exception {
public:
  ParseException() = default;
  ParseException(std::string err) : _error(std::move(err)) {}

  char const *what() const noexcept override { return _error.c_str(); }

private:
  std::string _error;
};

class LexException : public ParseException {
public:
  using ParseException::ParseException;
};

class SyntaxException : public ParseException {
public:
  using ParseException::ParseException;
  SyntaxException(size_t line, const char *err)
      : ParseException(std::format("line {}: {}", line, err)) {}
};

class AbstreeException : public ParseException {
public:
  using ParseException::ParseException;
};
} // namespace byfxxm

#endif
