#ifndef _BYFXXM_GPARSER_HPP_
#define _BYFXXM_GPARSER_HPP_

#include "address.hpp"
#include "ginterface.hpp"
#include "syntax.hpp"
#include <fstream>
#include <sstream>

namespace byfxxm {
class Gparser {
public:
  template <StreamConcept T>
  Gparser(T &&par_)
      : _gparser_impl(
            std::make_unique<_GparserImpl<T>>(std::forward<T>(par_))) {}

  std::optional<std::string>
  Run(Address *addr, Ginterface *gimpl,
      const std::function<void(const Snapshot &)> &update = {}) {
    return _gparser_impl->Run(addr, gimpl, update);
  }

private:
  class _GparserBase {
  public:
    virtual ~_GparserBase() = default;
    virtual std::optional<std::string>
    Run(Address *, Ginterface *,
        const std::function<void(const Snapshot &)> &) = 0;
  };

  template <StreamConcept T> class _GparserImpl : public _GparserBase {
  public:
    _GparserImpl(T &&stream) : _stream(std::move(stream)) {}

    std::optional<std::string>
    Run(Address *addr, Ginterface *gimpl,
        const std::function<void(const Snapshot &)> &update) override {
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

  std::unique_ptr<_GparserBase> _gparser_impl;
};
} // namespace byfxxm

#endif
