#ifndef _GPARSER_MEMORY_HPP_
#define _GPARSER_MEMORY_HPP_

#include <memory>
#include <memory_resource>
#include <type_traits>

namespace byfxxm {
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
  UniquePtr(UniquePtr<T1> &&rhs) noexcept : _pointer(std::move(rhs._pointer)) {}

  template <class T1> UniquePtr(T1 *p) noexcept : _pointer(p) {}

  template <class T1, class D1>
  UniquePtr(T1 *p, D1 &&del) noexcept : _pointer(p, std::forward<D1>(del)) {}

  explicit operator bool() const noexcept { return _pointer.operator bool(); }

  decltype(auto) operator->() const noexcept { return _pointer.operator->(); }

  decltype(auto) Reset(auto &&...pars) noexcept {
    return _pointer.reset(std::forward<decltype(pars)>(pars)...);
  }

  decltype(auto) Get() const noexcept { return _pointer.get(); }

  decltype(auto) operator*() const noexcept { return _pointer.operator*(); }

private:
  std::unique_ptr<T, D> _pointer;

  template <class T1, class D1> friend class UniquePtr;
  template <class T1> friend class ClonePtr;
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

template <class T> class ClonePtr {
public:
  ClonePtr() = default;
  ~ClonePtr() = default;
  ClonePtr(ClonePtr &&) noexcept = default;
  ClonePtr &operator=(ClonePtr &&) noexcept = default;

  template <class T1>
  ClonePtr(UniquePtr<T1> &&rhs) noexcept : _pointer(std::move(rhs)) {}

  ClonePtr(const UniquePtr<T> &rhs) {
    if (!rhs)
      return;

    if constexpr (std::is_abstract_v<T>) {
      _pointer = rhs->Clone();
    } else {
      if (rhs._pointer.get_deleter().mr_)
        _pointer = MakeUnique<T>(*rhs._pointer.get_deleter().mr_, *rhs);
      else
        _pointer = MakeUnique<T>(*rhs);
    }
  }

  ClonePtr(const ClonePtr &rhs) : ClonePtr(rhs._pointer) {}

  // copy and swap
  ClonePtr &operator=(const ClonePtr &rhs) {
    using std::swap;
    ClonePtr copy(rhs);
    swap(*this, copy);
    return *this;
  }

  explicit operator bool() const noexcept { return _pointer.operator bool(); }

  decltype(auto) operator->() const noexcept { return _pointer.operator->(); }

  decltype(auto) Reset(auto &&...pars) noexcept {
    return _pointer.Reset(std::forward<decltype(pars)>(pars)...);
  }

  decltype(auto) Get() const noexcept { return _pointer.Get(); }

private:
  UniquePtr<T> _pointer;
};

#ifdef __GNUC__
inline std::pmr::synchronized_pool_resource mempool;
#else
inline thread_local std::pmr::unsynchronized_pool_resource mempool;
#endif
} // namespace byfxxm

#endif
