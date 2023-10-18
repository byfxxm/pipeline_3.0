#pragma once
#include "coro.hpp"
#include "ring_buffer.hpp"
#include <memory>
#include <vector>

namespace byfxxm {
using Fifo = RingBuffer<std::unique_ptr<Code>, 4>;
struct Station {
  std::unique_ptr<Worker> worker;
  std::unique_ptr<Fifo> next = std::make_unique<Fifo>();
  Fifo *prev = nullptr;
  bool done = false;
};

struct PipelineException : public std::exception {
public:
  PipelineException() = default;
  PipelineException(std::string err) : _error(std::move(err)) {}

private:
  std::string _error;
};

class PipelineImp : public Pipeline {
public:
  void Start() override {
    if (_station_list.empty())
      return;

    _co.SetMain([this](CoMainHelper *helper, void *) {
      while (!_stop && _cur_station < _station_list.size()) {
        helper->SwitchToSub(_cur_station);
      }
    });

    for (auto &sta : _station_list) {
      _co.AddSub([&](CoSubHelper *helper, void *) {
        auto read = [&]() {
          std::unique_ptr<Code> code;
          while (!sta->prev->Read(code)) {
            if (_cur_station > 0 && _station_list[_cur_station - 1]->done) {
              sta->done = true;
              ++_cur_station;
            } else {
              --_cur_station;
            }
            helper->SwitchToMain();
          }

          return code;
        };

        auto write = [&](std::unique_ptr<Code> code) {
          while (!sta->next->Write(std::move(code))) {
            ++_cur_station;
            helper->SwitchToMain();
          }
        };

        try {
          if (sta->prev == nullptr) {
            if (!sta->worker->Do(nullptr, write)) {
              throw PipelineException();
            }
            write(nullptr);
          } else {
            for (;;) {
              auto code = read();
              if (!code)
                break;

              if (!sta->worker->Do(std::move(code), write))
                throw PipelineException();
            }
          }
        } catch (const PipelineException &) {
          Stop();
          helper->SwitchToMain();
          return;
        }

        sta->done = true;
        ++_cur_station;
        helper->SwitchToMain();
      });
    }

    _stop = false;
    _co.AsyncRun();
  }

  void Stop() override { _stop = true; }

  void Wait() override { _co.Wait(); }

  void AddWorker(std::unique_ptr<Worker> worker) override {
    if (!worker)
      throw PipelineException("worker is null");

    auto station = std::make_unique<Station>();
    station->worker = std::move(worker);
    if (!_station_list.empty())
      station->prev = _station_list.back()->next.get();

    _station_list.push_back(std::move(station));
  }

private:
  std::vector<std::unique_ptr<Station>> _station_list;
  size_t _cur_station = 0;
  Coro _co;
  std::atomic<bool> _stop = false;
};
} // namespace byfxxm
