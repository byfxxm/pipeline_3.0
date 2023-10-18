#pragma once
#include "ginterface.hpp"
#include "syntax.hpp"
#include "token.hpp"

namespace byfxxm {
template <StreamConcept T> class Gparser {
public:
  Gparser(T &&stream) : _syntax(std::move(stream)) {}
  Gparser(const std::string &str) : _syntax(std::istringstream(str)) {}
  Gparser(const std::filesystem::path &file) : _syntax(std::ifstream(file)) {}

  void Run(Address *addr, Ginterface *pimpl,
           std::function<void(size_t)> updateline = {}) {
    _syntax.SetEnv(addr, pimpl);
    while (auto abs_tree = _syntax.Next()) {
      auto &[tree, line] = abs_tree.value();
      if (updateline)
        updateline(line);

      tree.Execute();
    }
  }

private:
  Syntax<T> _syntax;
};

template <class T> Gparser(T) -> Gparser<T>;

Gparser(std::string) -> Gparser<std::istringstream>;
Gparser(std::filesystem::path) -> Gparser<std::ifstream>;
} // namespace byfxxm
