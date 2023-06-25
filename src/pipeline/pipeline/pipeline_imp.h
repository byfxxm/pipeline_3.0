#pragma once
#include <vector>
#include <memory>
#include "coro.h"
#include "worker.h"
#include "ring_buffer.h"

namespace byfxxm {
	using Fifo = RingBuffer<Code*, 4>;
	struct Station {
		Worker* worker = nullptr;
		std::unique_ptr<Fifo> next = std::make_unique<Fifo>();
		Fifo* prev = nullptr;
		bool done = false;
	};

	struct PipelineException : public std::exception {
	};

	class PipelineImp {
	public:
		void Start() {
			if (_station_list.empty())
				return;

			_co.SetMain([this](CoMainHelper* helper, void*) {
				while (!_stop && _cur_station < _station_list.size()) {
					helper->SwitchToSub(_cur_station);
				}
				});

			for (auto& sta : _station_list) {
				_co.AddSub([&](CoSubHelper* helper, void*) {
					auto read = [&]()->Code* {
						Code* code = nullptr;
						while (!sta->prev->Read(code)) {
							if (_cur_station > 0 && _station_list[_cur_station - 1]->done) {
								sta->done = true;
								++_cur_station;
							}
							else {
								--_cur_station;
							}
							helper->SwitchToMain();
						}

						return code;
					};

					auto write = [&](Code* code) {
						while (!sta->next->Write(code)) {
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
						}
						else {
							while (1) {
								Code* code = read();
								if (!code)
									break;

								if (!sta->worker->Do(code, write))
									throw PipelineException();
							}
						}
					}
					catch (const PipelineException&) {
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

		void Stop() {
			_stop = true;
		}

		void AddWorker(void* worker) {
			auto station = std::make_unique<Station>();
			station->worker = static_cast<Worker*>(worker);
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
}