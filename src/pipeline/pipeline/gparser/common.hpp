#ifndef _BYFXXM_TYPEDEFS_HPP_
#define _BYFXXM_TYPEDEFS_HPP_

#include "token.hpp"
#include <concepts>
#include <limits>
#include <memory_resource>
#include <variant>

namespace byfxxm {
template <class T>
concept StreamConcept = requires(T t) {
  { t.get() } -> std::integral;
  { t.peek() } -> std::integral;
  { t.eof() } -> std::same_as<bool>;
  T(std::move(t));
  requires std::is_convertible_v<T &, decltype(t.seekg(int64_t()))>;
  requires std::is_convertible_v<T &, decltype(t.unget())>;
};

template <class... Ts> struct Overloaded : Ts... {
  using Ts::operator()...;
};

struct Gtag {
  token::Kind code;
  double value;
};

inline constexpr bool operator==(const Gtag &lhs, const Gtag &rhs) {
  return lhs.code == rhs.code && lhs.value == rhs.value;
}

class SharpValue {
public:
  using GetSharp = std::function<double()>;
  using SetSharp = std::function<void(double)>;
  using GetSetSharp = std::tuple<GetSharp, SetSharp>;

  SharpValue(double *v) : _value(v) {}
  SharpValue(const GetSetSharp &f) : _value(f) {}

  [[nodiscard]] operator double() const {
    return std::visit(Overloaded{[](double *p) { return *p; },
                                 [](const GetSetSharp &getset) {
                                   return std::get<GetSharp>(getset)();
                                 }},
                      _value);
  }

  SharpValue &operator=(double val) {
    std::visit(Overloaded{[=](double *p) { *p = val; },
                          [=](const GetSetSharp &getset) {
                            std::get<SetSharp>(getset)(val);
                          }},
               _value);
    return *this;
  }

private:
  std::variant<double *, GetSetSharp> _value;
};

using Group = std::pmr::vector<double>;
using Value = std::variant<std::monostate, double, SharpValue, std::string,
                           bool, Gtag, Group>;
using GetRetVal = std::function<Value()>;

inline constexpr double nan = std::numeric_limits<double>::quiet_NaN();
inline constexpr bool IsNaN(double v) { return v != v; }

struct Snapshot {
  size_t line{};
  int64_t pos{};
};

using GetSnapshot = std::function<Snapshot()>;
using GotoSnapshot = std::function<void(const Snapshot &)>;
using MarkSnapshot = std::function<void(double)>;
using SnapshotTable = std::unordered_map<double, Snapshot>;

template <class T> struct Deleter {
  Deleter(std::pmr::memory_resource *mr = nullptr) : mr_(mr) {}

  template <std::derived_from<T> T1>
  Deleter(const Deleter<T1> &del) noexcept : size_(sizeof(T1)), mr_(del.mr_) {}

  void operator()(T *p) const {
    if (mr_) {
      p->~T();
      mr_->deallocate(p, size_);
    } else {
      delete p;
    }
  }

  size_t size_{sizeof(T)};
  std::pmr::memory_resource *mr_{nullptr};
};

template <class T, class D = Deleter<T>> class UniquePtr {
public:
  UniquePtr() = default;

  template <class T1>
    requires std::is_convertible_v<T1 *, T *>
  UniquePtr(UniquePtr<T1> &&rhs) noexcept : _pointer(std::move(rhs._pointer)) {}

  template <class T1>
    requires std::is_convertible_v<T1 *, T *>
  explicit UniquePtr(T1 *p) noexcept : _pointer(p) {}

  template <class T1, class D1>
    requires std::is_convertible_v<T1 *, T *>
  UniquePtr(T1 *p, D1 &&del) noexcept : _pointer(p, std::forward<D1>(del)) {}

  decltype(auto) reset(auto &&...pars) noexcept {
    return _pointer.reset(std::forward<decltype(pars)>(pars)...);
  }

  decltype(auto) get() const noexcept { return _pointer.get(); }

  explicit operator bool() const noexcept { return _pointer.operator bool(); }

  decltype(auto) operator->() const noexcept { return _pointer.operator->(); }

  decltype(auto) operator*() const noexcept { return _pointer.operator*(); }

private:
  std::unique_ptr<T, D> _pointer;
  template <class T1, class D1> friend class UniquePtr;
};

template <class T, class... Args>
  requires std::is_constructible_v<T, Args...>
[[nodiscard]] auto MakeUnique(Args &&...args) {
  return UniquePtr<T>(new T(std::forward<Args>(args)...));
}

template <class T, class... Args>
  requires std::is_constructible_v<T, Args...>
[[nodiscard]] auto MakeUnique(std::pmr::memory_resource &mr, Args &&...args) {
  return UniquePtr<T>(new (mr.allocate(sizeof(T)))
                          T(std::forward<Args>(args)...),
                      Deleter<T>(&mr));
}

#ifdef __GNUC__
inline std::pmr::synchronized_pool_resource mempool;
#else
inline thread_local std::pmr::unsynchronized_pool_resource mempool;
#endif
} // namespace byfxxm

#endif
