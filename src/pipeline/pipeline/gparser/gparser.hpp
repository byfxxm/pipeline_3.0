#ifndef _BYFXXM_GPARSER_HPP_
#define _BYFXXM_GPARSER_HPP_

#include "address.hpp"
#include "ginterface.hpp"
#include "syntax.hpp"
#include <fstream>
#include <sstream>

namespace byfxxm {
template <StreamConcept T> class Gparser {
public:
  Gparser(T &&stream) : _stream(std::move(stream)) {}
  Gparser(const std::string &str) : _stream(std::istringstream(str)) {}
  Gparser(const std::filesystem::path &file) : _stream(std::ifstream(file)) {}

  std::optional<std::string>
  Run(Address *addr, Ginterface *gimpl,
      std::function<void(const Snapshot &)> update = {}) {
    std::optional<std::string> ret;
    try {
      Syntax<T> syn(std::move(_stream), addr, gimpl);
      while (auto abstree = syn.Next()) {
        auto &[tree, snapshot] = abstree.value();
        if (update)
          update(snapshot);

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
