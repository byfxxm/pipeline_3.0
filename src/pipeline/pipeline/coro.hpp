#ifndef _CORO_HPP_
#define _CORO_HPP_

#include <Windows.h>
#include <future>
#include <memory>
#include <tuple>
#include <utility>

namespace byfxxm {
class CoMainHelper;
class CoSubHelper;
using _CoMainFunc = std::function<void(CoMainHelper *, void *)>;
using _CoSubFunc = std::function<void(CoSubHelper *, void *)>;
using _CoUserPtr = void *;
using _CoPtr = std::unique_ptr<void, std::function<void(void *)>>;
using _CoMain = std::tuple<_CoMainFunc, _CoUserPtr, _CoPtr>;
using _CoSub = std::tuple<_CoSubFunc, _CoUserPtr, _CoPtr>;

class CoMainHelper {
public:
  CoMainHelper(const std::vector<_CoSub> &co_list) : _co_subs(co_list) {}

  void SwitchToSub(size_t n) {
    SwitchToFiber(std::get<_CoPtr>(_co_subs[n]).get());
  }

private:
  const std::vector<_CoSub> &_co_subs;
};

class CoSubHelper {
public:
  CoSubHelper(const _CoMain &co) : _co_main(co) {}

  void SwitchToMain() { SwitchToFiber(std::get<_CoPtr>(_co_main).get()); }

private:
  const _CoMain &_co_main;
};

class Coro {
public:
  Coro() = default;
  ~Coro() {
    if (_runtime.joinable())
      _runtime.join();
  }

  void SetMain(const std::function<void(CoMainHelper *, void *)> &func,
               void *user = nullptr) {
    _co_main = std::make_tuple(func, user, nullptr);
  }

  void AddSub(const std::function<void(CoSubHelper *, void *)> &func,
              void *user = nullptr) {
    _co_subs.emplace_back(func, user, nullptr);
  }

  void AsyncRun() {
    if (_runtime.joinable())
      return;

    _runtime = std::thread([this] { _DoRun(); });
  }

  void Wait() {
    if (_runtime.joinable())
      _runtime.join();
  }

  void Run() {
    if (_runtime.joinable())
      return;

    _runtime = std::thread([this] { _DoRun(); });

    _runtime.join();
  }

private:
  void _DoRun() {
    CoSubHelper sub_helper(_co_main);
    std::vector<std::unique_ptr<std::packaged_task<void()>>> co_binds(
        _co_subs.size());
    for (size_t i = 0; i < _co_subs.size(); ++i) {
      auto &co = _co_subs[i];
      auto &co_bind = co_binds[i];
      co_bind = std::make_unique<std::packaged_task<void()>>(std::bind(
          std::get<_CoSubFunc>(co), &sub_helper, std::get<_CoUserPtr>(co)));

      auto fiber = CreateFiber(
          0, [](void *p) { (*static_cast<std::packaged_task<void()> *>(p))(); },
          co_bind.get());

      std::get<_CoPtr>(co) = _CoPtr(fiber, [](void *p) { DeleteFiber(p); });
    }

    auto main_fiber =
        std::unique_ptr<void, decltype([](void *) { ConvertFiberToThread(); })>(
            ConvertThreadToFiber(nullptr));
    CoMainHelper main_helper(_co_subs);
    std::get<_CoPtr>(_co_main) = _CoPtr(main_fiber.get(), [](void *) {});
    std::get<_CoMainFunc>(_co_main)(&main_helper,
                                    std::get<_CoUserPtr>(_co_main));
  }

private:
  _CoMain _co_main;
  std::vector<_CoSub> _co_subs;
  std::thread _runtime;
};
} // namespace byfxxm

#endif
