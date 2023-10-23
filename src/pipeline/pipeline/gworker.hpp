#pragma once
#include "code.hpp"
#include "gparser/gparser.hpp"
#include "worker.hpp"
#include <format>
#include <type_traits>

namespace byfxxm {
inline void print_gparams(std::string str, const Gparams &params) {
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

inline AxesArray GparamsToEnd(const Gparams &params) {
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

inline AxesArray GparamsToCenter(const Gparams &params) {
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

class Gpimpl : public Ginterface {
public:
  Gpimpl(const WriteFunc &writefn) : _writefn(writefn) {}

  virtual bool None(const Gparams &params, const Address *addr) override {
    if (_last == Gtag{token::Kind::G, 0})
      _writefn(std::make_unique<Move>(GparamsToEnd(params)));
    else if (_last == Gtag{token::Kind::G, 1})
      _writefn(std::make_unique<Line>(GparamsToEnd(params)));
    else if (_last == Gtag{token::Kind::G, 2})
      _writefn(std::make_unique<Arc>(GparamsToEnd(params),
                                     GparamsToCenter(params), false));
    else if (_last == Gtag{token::Kind::G, 3})
      _writefn(std::make_unique<Arc>(GparamsToEnd(params),
                                     GparamsToCenter(params), true));

    return true;
  }

  virtual bool G0(const Gparams &params, const Address *addr) override {
    _last = {token::Kind::G, 0};
    _writefn(std::make_unique<Move>(GparamsToEnd(params)));
    return true;
  }

  virtual bool G1(const Gparams &params, const Address *addr) override {
    _last = {token::Kind::G, 1};
    _writefn(std::make_unique<Line>(GparamsToEnd(params)));
    return true;
  }

  virtual bool G2(const Gparams &params, const Address *addr) override {
    _last = {token::Kind::G, 2};
    _writefn(std::make_unique<Arc>(GparamsToEnd(params),
                                   GparamsToCenter(params), false));
    return true;
  }

  virtual bool G3(const Gparams &params, const Address *addr) override {
    _last = {token::Kind::G, 3};
    _writefn(std::make_unique<Arc>(GparamsToEnd(params),
                                   GparamsToCenter(params), true));
    return true;
  }

  virtual bool G4(const Gparams &params, const Address *addr) override {
    print_gparams("G4", params);
    return true;
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
    std::visit(
        [&](auto &&parser) {
          if constexpr (std::is_same_v<std::monostate,
                                       std::decay_t<decltype(parser)>>)
            assert(0);
          else {
            Address addr;
            Gpimpl gpimpl(writefn);
            parser.Run(&addr, &gpimpl);
          }
        },
        _parser);
    return true;
  }

private:
  std::variant<std::monostate, Gparser<std::fstream>,
               Gparser<std::stringstream>>
      _parser;
};
} // namespace byfxxm
