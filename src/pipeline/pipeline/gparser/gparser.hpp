#ifndef _GPARSER_HPP_
#define _GPARSER_HPP_

#include "ginterface.hpp"
#include "syntax.hpp"
#include "token.hpp"

namespace byfxxm {
template <StreamConcept T> class Gparser {
public:
  Gparser(T &&stream) : _stream(std::move(stream)) {}
  Gparser(const std::string &str) : _stream(std::istringstream(str)) {}
  Gparser(const std::filesystem::path &file) : _stream(std::ifstream(file)) {}

  void Run(Address *addr, Ginterface *pimpl,
           std::function<void(size_t)> updateline = {}) {
    Syntax<T> syntax(std::move(_stream), addr, pimpl);
    while (auto abs_tree = syntax.Next()) {
      auto &[tree, line] = abs_tree.value();
      if (updateline)
        updateline(line);

      tree.Execute();
    }
  }

private:
  T _stream;
};

template <class T> Gparser(T) -> Gparser<T>;

Gparser(std::string) -> Gparser<std::istringstream>;
Gparser(std::filesystem::path) -> Gparser<std::ifstream>;
} // namespace byfxxm

#endif
