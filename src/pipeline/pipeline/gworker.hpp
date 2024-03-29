﻿#ifndef _BYFXXM_GWORKER_HPP_
#define _BYFXXM_GWORKER_HPP_

#include "code.hpp"
#include "gparser/gparser.hpp"
#include "worker.hpp"
#include <format>
#include <type_traits>

namespace byfxxm {
inline void print_gparams(std::string str, const Ginterface::Params &params) {
  static const std::unordered_map<token::Kind, std::string> map = {
      {token::Kind::X, "X"},
      {token::Kind::Y, "Y"},
      {token::Kind::Z, "Z"},
  };

  std::ranges::for_each(params, [&](const Gtag &item) {
    str += std::format(" {}{}", map.at(item.code), item.value);
  });
  puts(str.c_str());
}

inline AxesArray GparamsToEnd(const Ginterface::Params &params) {
  AxesArray ret(6);
  ret.Memset(nan);
  std::ranges::for_each(params, [&](const Gtag &item) {
    switch (item.code) {
    case token::Kind::X:
      ret[0] = item.value;
      break;
    case token::Kind::Y:
      ret[1] = item.value;
      break;
    case token::Kind::Z:
      ret[2] = item.value;
      break;
    default:
      break;
    }
  });

  return ret;
}

inline AxesArray GparamsToCenter(const Ginterface::Params &params) {
  AxesArray ret(6);
  ret.Memset(nan);
  std::ranges::for_each(params, [&](const Gtag &item) {
    switch (item.code) {
    case token::Kind::I:
      ret[0] = item.value;
      break;
    case token::Kind::J:
      ret[1] = item.value;
      break;
    case token::Kind::K:
      ret[2] = item.value;
      break;
    default:
      break;
    }
  });

  return ret;
}

class Gimpl : public Ginterface {
public:
  Gimpl(const WriteFunc &writefn) : _writefn(writefn) {}

  virtual void None(const Utils &utils) override {
    if (_last == Gtag{token::Kind::G, 0})
      _writefn(std::make_unique<Move>(GparamsToEnd(utils.params)));
    else if (_last == Gtag{token::Kind::G, 1})
      _writefn(std::make_unique<Line>(GparamsToEnd(utils.params)));
    else if (_last == Gtag{token::Kind::G, 2})
      _writefn(std::make_unique<Arc>(GparamsToEnd(utils.params),
                                     GparamsToCenter(utils.params), false));
    else if (_last == Gtag{token::Kind::G, 3})
      _writefn(std::make_unique<Arc>(GparamsToEnd(utils.params),
                                     GparamsToCenter(utils.params), true));
  }

  virtual void G0(const Utils &utils) override {
    _last = {token::Kind::G, 0};
    _writefn(std::make_unique<Move>(GparamsToEnd(utils.params)));
  }

  virtual void G1(const Utils &utils) override {
    _last = {token::Kind::G, 1};
    _writefn(std::make_unique<Line>(GparamsToEnd(utils.params)));
  }

  virtual void G2(const Utils &utils) override {
    _last = {token::Kind::G, 2};
    _writefn(std::make_unique<Arc>(GparamsToEnd(utils.params),
                                   GparamsToCenter(utils.params), false));
  }

  virtual void G3(const Utils &utils) override {
    _last = {token::Kind::G, 3};
    _writefn(std::make_unique<Arc>(GparamsToEnd(utils.params),
                                   GparamsToCenter(utils.params), true));
  }

  virtual void G4(const Utils &utils) override {
    print_gparams("G4", utils.params);
  }

  virtual void N(const Utils &utils) override {
    print_gparams("N", utils.params);
  }

private:
  Gtag _last{token::Kind::G, 0};
  WriteFunc _writefn;
};

class Gworker : public Worker {
public:
  Gworker(StreamConcept auto &&stream)
      : _parser(std::forward<decltype(stream)>(stream)) {}

private:
  virtual bool Do(std::unique_ptr<Code>,
                  const WriteFunc &writefn) noexcept override {
    Address addr;
    Gimpl gimpl(writefn);
    _parser.Run(&addr, &gimpl);
    return true;
  }

private:
  Gparser _parser;
};
} // namespace byfxxm

#endif
