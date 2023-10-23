#ifndef _EXCEPTION_HPP_
#define _EXCEPTION_HPP_

#include <exception>
#include <string>

namespace byfxxm {
class ParseException : public std::exception {
public:
  ParseException() = default;
  ParseException(std::string err) : _error(std::move(err)) {}

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
      : ParseException(std::to_string(line) + " : " + err) {}
};

class AbstreeException : public ParseException {
public:
  using ParseException::ParseException;
};
} // namespace byfxxm

#endif
