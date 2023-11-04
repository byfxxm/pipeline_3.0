#ifndef _BYFXXM_GPARSER_HPP_
#define _BYFXXM_GPARSER_HPP_

#include "ginterface.hpp"
#include "syntax.hpp"
#include "token.hpp"
#include <fstream>
#include <sstream>

namespace byfxxm {
template <StreamConcept T> class Gparser {
public:
  Gparser(T &&stream) : _stream(std::move(stream)) {}
  Gparser(const std::string &str) : _stream(std::istringstream(str)) {}
  Gparser(const std::filesystem::path &file) : _stream(std::ifstream(file)) {}

  std::optional<std::string> Run(Address *addr, Ginterface *pimpl,
                                 std::function<void(size_t)> updateline = {}) {
    std::optional<std::string> ret;
    try {
      Syntax<T> syn(std::move(_stream), addr, pimpl);
      while (auto abstree = syn.Next()) {
        auto &[tree, line] = abstree.value();
        if (updateline)
          updateline(line);

        tree();
      }
    } catch (const ParseException &ex) {
      ret = std::format("#error: {}", ex.what());
    }

    return ret;
  }

private:
  T _stream;
};

template <class T> Gparser(T) -> Gparser<T>;
Gparser(std::string) -> Gparser<std::istringstream>;
Gparser(std::filesystem::path) -> Gparser<std::ifstream>;
} // namespace byfxxm

#endif
